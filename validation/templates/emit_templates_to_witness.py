#!/usr/bin/env python3

import argparse
import copy
import json
import pathlib
import re
import subprocess
import sys
import tempfile
from typing import Dict, List, Optional, Tuple


ROOT = pathlib.Path(__file__).resolve().parent
DEFAULT_CPPGM = ROOT.parents[1] / "dev" / "cppgm++"

EVENT_HEADER_RE = re.compile(r"^(class-use|alias-use|variable-use|function-call) at (.+)$")
CLOSURE_EVENT_HEADER_RE = re.compile(r"^([a-z-]+)(?: at (.+))?$")
BIND_RE = re.compile(r"^(bind|specialize) (.+?) = (.*) source=([A-Za-z0-9_]+)$")
DROP_RE = re.compile(r"^drop (.+?) at (.+?) reason=(.+)$")
IDENT_CHAR_RE = re.compile(r"[A-Za-z0-9_]")
DECL_PREFIX_RE = re.compile(r".*\b(?:class|struct|union)\s*$")
ASSIGNMENT_SOURCE_PATH_RE = re.compile(
    r"(?:^|/)pa[0-9]+/((?:tests|course)/.*)$")

STD_SOURCE_ALIASES = {
    "std::basic_string": "std::string",
    "std::basic_istringstream": "std::istringstream",
    "std::basic_ostringstream": "std::ostringstream",
    "std::basic_string_view": "std::string_view",
}

STD_DECL_LOCATION_OVERRIDES = {
    ("class_use", "std::basic_istringstream"): "libc++/__fwd/sstream.h:26:7",
    ("class_use", "std::basic_string"): "libc++/__fwd/string.h:43:7",
    ("function_call", "std::getline"): "libc++/istream:1342:1",
    ("function_call", "std::operator>>"): "libc++/istream:1218:1",
}

PRIMARY_BINDING_ORDER = {
    "t": 0,
    "_tp": 0,
    "_key": 0,
    "_chart": 0,
    "_rp": 0,
    "callback": 0,
    "fn": 0,
    "u": 0,
    "_traits": 1,
    "_compare": 1,
    "_pred": 1,
    "_allocator": 2,
    "_alloc": 2,
}


def normalize_witness_path(path: str) -> str:
    value = path.replace("\\", "/")
    libcxx_marker = "/include/c++/v1/"
    libcxx_pos = value.find(libcxx_marker)
    if libcxx_pos != -1:
        return "libc++/" + value[libcxx_pos + len(libcxx_marker):]
    assignment_match = ASSIGNMENT_SOURCE_PATH_RE.search(value)
    if assignment_match:
        return assignment_match.group(1)
    return value


def normalize_witness_location(location: str) -> str:
    parts = location.rsplit(":", 2)
    if len(parts) != 3 or not parts[1].isdigit() or not parts[2].isdigit():
        return location
    return f"{normalize_witness_path(parts[0])}:{parts[1]}:{parts[2]}"


def split_top_level(text: str, separator: str = ",") -> List[str]:
    out: List[str] = []
    current: List[str] = []
    depth_angle = 0
    depth_paren = 0
    depth_brace = 0
    depth_bracket = 0
    for ch in text:
        if ch == "<":
            depth_angle += 1
        elif ch == ">" and depth_angle > 0:
            depth_angle -= 1
        elif ch == "(":
            depth_paren += 1
        elif ch == ")" and depth_paren > 0:
            depth_paren -= 1
        elif ch == "{":
            depth_brace += 1
        elif ch == "}" and depth_brace > 0:
            depth_brace -= 1
        elif ch == "[":
            depth_bracket += 1
        elif ch == "]" and depth_bracket > 0:
            depth_bracket -= 1

        if ch == separator and depth_angle == 0 and depth_paren == 0 and \
                depth_brace == 0 and depth_bracket == 0:
            out.append("".join(current).strip())
            current = []
            continue
        current.append(ch)
    out.append("".join(current).strip())
    return out


def find_matching_angle(text: str, open_index: int) -> Optional[int]:
    depth = 0
    for index in range(open_index, len(text)):
        ch = text[index]
        if ch == "<":
            depth += 1
        elif ch == ">":
            depth -= 1
            if depth == 0:
                return index
    return None


def is_token_boundary(text: str, index: int) -> bool:
    if index <= 0:
        return True
    return IDENT_CHAR_RE.match(text[index - 1]) is None


def find_spelling_occurrences(lines: List[str], spelling: str) -> List[Dict]:
    occurrences: List[Dict] = []
    if not spelling:
        return occurrences
    for line_no, line in enumerate(lines, 1):
        start = 0
        while True:
            pos = line.find(spelling, start)
            if pos == -1:
                break
            if not is_token_boundary(line, pos):
                start = pos + 1
                continue
            after = pos + len(spelling)
            args: List[str] = []
            if after < len(line) and line[after] == "<":
                close = find_matching_angle(line, after)
                if close is not None:
                    args = split_top_level(line[after + 1:close])
            occurrences.append({
                "location": f"{line_no}:{pos + 1}",
                "args": args,
            })
            start = pos + 1
    return occurrences


def find_template_id_occurrences(lines: List[str], spelling: str) -> List[Dict]:
    return [occurrence for occurrence in find_spelling_occurrences(lines, spelling)
            if occurrence.get("args")]


def template_source_spellings(event: Dict) -> List[str]:
    template_name = event.get("template") or ""
    if not template_name:
        return []
    spellings: List[str] = []
    bindings = event.get("bindings", [])
    if bindings:
      args = [binding.get("arg", "") for binding in bindings if binding.get("arg", "")]
      for count in range(len(args), 0, -1):
          spellings.append(f"{template_name}<{', '.join(args[:count])}>")
    resolved = event.get("resolved") or ""
    if resolved:
        spellings.append(resolved)
    spellings.append(template_name)
    if template_name in STD_SOURCE_ALIASES:
        spellings.insert(0, STD_SOURCE_ALIASES[template_name])
    if "::" in template_name:
        unqualified = template_name.split("::")[-1]
        extras: List[str] = []
        for spelling in list(spellings):
            extras.append(spelling.replace(template_name, unqualified))
        spellings.extend(extras)
    deduped: List[str] = []
    seen = set()
    for spelling in spellings:
        if not spelling or spelling in seen:
            continue
        seen.add(spelling)
        deduped.append(spelling)
    return deduped


def source_location_for_occurrence(input_path: pathlib.Path, location: str) -> str:
    return f"{input_path}:{location}"


def strip_namespace_qualifiers(text: str) -> str:
    return re.sub(r"\b[A-Za-z_][A-Za-z0-9_]*::", "", text)


def normalize_const_order(text: str) -> str:
    value = text
    patterns = [
        (r"\b([A-Za-z_][A-Za-z0-9_:]*)\s+const(\s*[*&])", r"const \1\2"),
        (r"\b([A-Za-z_][A-Za-z0-9_:]*)\s+const\b", r"const \1"),
    ]
    changed = True
    while changed:
        changed = False
        for pattern, replacement in patterns:
            updated = re.sub(pattern, replacement, value)
            if updated != value:
                value = updated
                changed = True
    return value


def normalize_public_type_spellings(text: str) -> str:
    value = normalize_const_order(normalize_local_entity_locations(text))
    value = re.sub(r"\b([0-9]+)[uUlL]+\b", r"\1", value)
    while True:
        updated = re.sub(r"\*\s+\*", r"**", value)
        if updated == value:
            break
        value = updated
    value = re.sub(r"\*\s+const\b", r"*const", value)
    value = re.sub(r"\*\s+volatile\b", r"*volatile", value)
    return value


def normalize_template_log_type_spellings(text: str) -> str:
    value = re.sub(r"__local_\d+", "", text)
    replacements = [
        ("unsigned long long int", "unsigned long long"),
        ("unsigned long int", "unsigned long"),
        ("long long int", "long long"),
        ("long int", "long"),
        ("short int", "short"),
        ("unsigned int", "unsigned"),
        ("signed int", "signed"),
    ]
    for old, new in replacements:
        value = value.replace(old, new)
    return normalize_public_type_spellings(value)


def collapse_duplicate_owner_prefix(entity: str) -> str:
    member_pos = entity.rfind("::")
    if member_pos == -1:
        return entity
    owner = entity[:member_pos]
    member = entity[member_pos + 2:]
    split = owner.find("::")
    while split != -1:
        owner_prefix = owner[:split]
        owner_suffix = owner[split + 2:]
        if owner_prefix == owner_suffix:
            return f"{owner_suffix}::{member}"
        split = owner.find("::", split + 2)
    return entity


def normalize_local_entity_locations(entity: str) -> str:
    # Embedded local-entity locations are part of anonymous/lambda type names,
    # not source navigation. Match cppgm's anonymous entity policy by keeping
    # only line:column and never baking checkout paths into public refs.
    def replace(match: re.Match) -> str:
        return f" at {match.group(1)}:{match.group(2)})"

    return re.sub(
        r" at [^()]*?([0-9]+):([0-9]+)\)",
        replace,
        entity)


def normalize_template_log_entity(entity: str) -> str:
    return normalize_template_log_type_spellings(
        normalize_local_entity_locations(
            collapse_duplicate_owner_prefix(entity)))


def inline_namespace_names(lines: List[str]) -> List[str]:
    names: List[str] = []
    seen = set()
    pattern = re.compile(r"\binline\s+namespace\s+([A-Za-z_][A-Za-z0-9_]*)\b")
    for line in lines:
        match = pattern.search(line)
        if not match:
            continue
        name = match.group(1)
        if name in seen:
            continue
        seen.add(name)
        names.append(name)
    return names


def strip_inline_namespace_segments(text: str, names: List[str]) -> str:
    value = text
    for name in names:
        value = value.replace(f"::{name}::", "::")
    return value


def canonical_template_text(text: str) -> str:
    value = normalize_const_order(strip_namespace_qualifiers(text)).replace(" ", "")
    value = value.replace("longint", "long")
    value = value.replace("unsignedint", "unsigned")
    value = value.replace("signedint", "signed")
    value = value.replace("longlongint", "longlong")
    return value


def extract_template_args(text: str) -> List[str]:
    open_index = text.find("<")
    if open_index == -1:
        return []
    close_index = find_matching_angle(text, open_index)
    if close_index is None:
        return []
    return split_top_level(text[open_index + 1:close_index])


def event_expected_args(event: Dict) -> List[str]:
    resolved = event.get("resolved") or ""
    args = extract_template_args(resolved)
    if args:
        return args
    return [binding.get("arg", "") for binding in event.get("bindings", []) if binding.get("arg", "")]


def occurrence_matches_event(event: Dict, occurrence: Dict) -> bool:
    if event.get("kind") != "class_use":
        return True
    args = occurrence.get("args") or []
    if not args:
        return False
    expected = event_expected_args(event)
    if not expected or len(args) > len(expected):
        return False
    for index, arg in enumerate(args):
        actual = canonical_template_text(arg)
        wanted = canonical_template_text(expected[index])
        if actual == wanted:
            continue
        if re.match(r"^\d+$", wanted) and re.search(r"[A-Za-z_+\-*/]", arg):
            continue
        if re.match(r"^\d+$", wanted) and actual.startswith("int:") and actual.split(":", 1)[1] == wanted:
            continue
        if actual != wanted:
            return False
    return True


def normalize_nested_member_decl_location(event: Dict,
                                          input_path: pathlib.Path,
                                          lines: List[str]) -> None:
    if event.get("kind") != "class_use":
        return
    template_name = event.get("template") or ""
    if "::" not in template_name:
        return
    decl = event.get("selected_decl_location") or ""
    if not decl.startswith(str(input_path)):
        return
    unqualified = template_name.split("::")[-1]
    for line_no, line in enumerate(lines, 1):
        for keyword in ("struct ", "class "):
            token = keyword + unqualified
            pos = line.find(token)
            if pos != -1:
                event["selected_decl_location"] = f"{input_path}:{line_no}:{pos + len(keyword) + 1}"
                return


def unqualified_selected_name(event: Dict) -> str:
    selected = event.get("selected") or event.get("template") or ""
    if not selected:
        return ""
    base = selected.split("::")[-1]
    if "<" in base:
        base = base.split("<", 1)[0]
    return base


def find_named_call_occurrences(lines: List[str], name: str) -> List[str]:
    if not name or name.startswith("operator") or name.startswith("~"):
        return []
    token = name
    occurrences: List[str] = []
    for line_no, line in enumerate(lines, 1):
        start = 0
        while True:
            pos = line.find(token, start)
            if pos == -1:
                break
            if not is_token_boundary(line, pos):
                start = pos + 1
                continue
            after = pos + len(token)
            if after < len(line) and line[after] == "<":
                close = find_matching_angle(line, after)
                if close is not None:
                    after = close + 1
            while after < len(line) and line[after].isspace():
                after += 1
            if after < len(line) and line[after] == "(":
                occurrences.append(f"{line_no}:{pos + 1}")
            start = pos + 1
    return occurrences


def find_specific_call_occurrences(lines: List[str], spelling: str) -> List[str]:
    occurrences: List[str] = []
    if not spelling:
        return occurrences
    for line_no, line in enumerate(lines, 1):
        start = 0
        while True:
            pos = line.find(spelling, start)
            if pos == -1:
                break
            if not is_token_boundary(line, pos):
                start = pos + 1
                continue
            after = pos + len(spelling)
            if after < len(line) and line[after] == "<":
                close = find_matching_angle(line, after)
                if close is not None:
                    after = close + 1
            while after < len(line) and line[after].isspace():
                after += 1
            if after < len(line) and line[after] == "(":
                occurrences.append(f"{line_no}:{pos + 1}")
            start = pos + 1
    return occurrences


def parse_line_col(location: Optional[str]) -> Tuple[int, int]:
    if not location or ":" not in location:
        return (0, 0)
    parts = location.rsplit(":", 2)
    if len(parts) == 3:
        _path, line_s, col_s = parts
    else:
        line_s, _, col_s = location.partition(":")
    try:
        return (int(line_s), int(col_s))
    except ValueError:
        return (0, 0)


def choose_call_occurrence(raw_location: Optional[str], occurrences: List[str]) -> Optional[str]:
    if not occurrences:
        return None
    raw_line, raw_col = parse_line_col(raw_location)
    parsed_occurrences = [(parse_line_col(occ), occ) for occ in occurrences]
    same_line = [item for item in parsed_occurrences if item[0][0] == raw_line and item[0][1] <= raw_col]
    if same_line:
        same_line.sort()
        return same_line[-1][1]
    return parsed_occurrences[-1][1]


def choose_first_same_line_occurrence(raw_location: Optional[str], occurrences: List[str]) -> Optional[str]:
    raw_line, _ = parse_line_col(raw_location)
    parsed_occurrences = [(parse_line_col(occ), occ) for occ in occurrences]
    same_line = [item for item in parsed_occurrences if item[0][0] == raw_line]
    if same_line:
        same_line.sort()
        return same_line[0][1]
    return None


def extract_call_arguments(line: str, column: int) -> List[str]:
    start = line.find("(", max(0, column - 1))
    if start == -1:
        return []
    depth = 0
    for index in range(start, len(line)):
        ch = line[index]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return split_top_level(line[start + 1:index])
    return []


def strip_enclosing_parens(text: str) -> str:
    value = text.strip()
    while value.startswith("(") and value.endswith(")"):
        inner = value[1:-1].strip()
        if not inner:
            break
        value = inner
    return value


def infer_identifier_type(lines: List[str], identifier: str, line_no: int) -> Optional[str]:
    escaped = re.escape(identifier)
    patterns = [
        re.compile(rf"^\s*(?P<type>.+?)\s+(?P<name>{escaped})\s*(?:[=;,\)])"),
        re.compile(rf"^\s*(?P<type>.+?)\s+(?P<name>{escaped})\s*(?:\[[^\]]*\])?\s*(?:[=;,\)])"),
    ]
    for current_line in range(min(line_no, len(lines)), 0, -1):
        text = lines[current_line - 1].strip()
        if not text or text.startswith("//"):
            continue
        for pattern in patterns:
            match = pattern.match(text)
            if not match:
                continue
            type_text = match.group("type").strip()
            if any(keyword in type_text for keyword in ("return", "if", "while", "for", "switch")):
                continue
            return type_text
    return None


def infer_expression_type(expr: str, lines: List[str], call_line_no: int) -> Optional[str]:
    value = strip_enclosing_parens(expr)
    if not value:
        return None
    if value.startswith("&"):
        pointee = infer_expression_type(value[1:].strip(), lines, call_line_no)
        if pointee:
            return pointee + " *"
    if re.match(r"^'(?:\\.|[^'])+'$", value):
        return "char"
    if re.match(r'^"(?:\\.|[^"])*"$', value):
        return "const char *"
    if value in ("true", "false"):
        return "bool"
    integer_match = re.match(r"^([0-9]+)([uUlL]*)$", value)
    if integer_match:
        suffix = integer_match.group(2).lower()
        if "ll" in suffix and "u" in suffix:
            return "unsigned long long"
        if "ll" in suffix:
            return "long long"
        if "l" in suffix and "u" in suffix:
            return "unsigned long"
        if "l" in suffix:
            return "long"
        if "u" in suffix:
            return "unsigned"
        return "int"
    if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", value):
        return infer_identifier_type(lines, value, call_line_no)
    return None


def template_declares_parameter_pack(lines: List[str],
                                     decl_location: Optional[str],
                                     param: str) -> bool:
    decl_line_no, _ = parse_line_col(decl_location)
    if decl_line_no <= 0:
        return False
    begin = max(1, decl_line_no - 3)
    for current_line in range(begin, decl_line_no + 1):
        text = lines[current_line - 1]
        if re.search(r"\.\.\.\s*" + re.escape(param) + r"\b", text):
            return True
    return False


def function_parameter_prefix_count_for_pack(lines: List[str],
                                             decl_location: Optional[str],
                                             param: str) -> Optional[int]:
    decl_line_no, _ = parse_line_col(decl_location)
    if decl_line_no <= 0 or decl_line_no > len(lines):
        return None
    text = ""
    for current_line in range(decl_line_no, min(len(lines), decl_line_no + 3) + 1):
        text += lines[current_line - 1].strip() + " "
        if ")" in lines[current_line - 1]:
            break
    open_index = text.find("(")
    if open_index == -1:
        return None
    depth = 0
    close_index = -1
    for index in range(open_index, len(text)):
        ch = text[index]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                close_index = index
                break
    if close_index == -1:
        return None
    params = split_top_level(text[open_index + 1:close_index])
    for index, entry in enumerate(params):
        if re.search(r"\.\.\.\s*" + re.escape(param) + r"\b", entry):
            return index
    return None


def is_plausible_function_call_event(event: Dict, lines: List[str]) -> bool:
    raw_location = event.get("raw_location") or event.get("location")
    line_no, _ = parse_line_col(raw_location)
    if line_no <= 0 or line_no > len(lines):
        return True
    text = lines[line_no - 1]
    selected = event.get("selected") or event.get("template") or ""
    if selected.endswith("operator<<"):
        return "<<" in text
    if selected.endswith("operator>>"):
        return ">>" in text
    if selected.endswith("operator="):
        return "=" in text and "==" not in text
    name = unqualified_selected_name(event)
    if not name or name.startswith("operator") or name.startswith("~"):
        return True
    return name in text and "(" in text


def normalize_function_decl_location(event: Dict,
                                     input_path: pathlib.Path,
                                     lines: List[str]) -> None:
    if event.get("kind") != "function_call":
        return
    decl = event.get("selected_decl_location") or ""
    if not decl.startswith(str(input_path)):
        return
    raw_decl = event.get("raw_selected_decl_location") or decl
    raw_line, _ = parse_line_col(raw_decl)
    name = unqualified_selected_name(event)
    if not name or name.startswith("operator") or name.startswith("~"):
        return
    occurrences = find_named_call_occurrences(lines, name)
    for occurrence in occurrences:
        line_no, col_no = parse_line_col(occurrence)
        if line_no < raw_line:
            continue
        if line_no > raw_line + 4:
            break
        event["selected_decl_location"] = f"{input_path}:{line_no}:{col_no}"
        return


def normalize_explicit_specialization_function_call(event: Dict,
                                                    input_path: pathlib.Path,
                                                    lines: List[str]) -> None:
    if event.get("kind") != "function_call":
        return
    if event.get("selection") != "instantiation":
        return
    selected_name = unqualified_selected_name(event)
    if not selected_name or selected_name.startswith("operator"):
        return
    binding_args = [
        binding.get("arg", "")
        for binding in event.get("bindings", [])
        if binding.get("param") and
        not binding.get("param", "").startswith("$") and
        binding.get("arg")
    ]
    if not binding_args:
        return
    wanted_args = [canonical_template_text(arg) for arg in binding_args]
    for line_no in range(1, len(lines) + 1):
        if "template<>" not in lines[line_no - 1]:
            continue
        for decl_line in range(line_no + 1, min(len(lines), line_no + 3) + 1):
            text = lines[decl_line - 1]
            token = selected_name + "<"
            pos = text.find(token)
            if pos == -1:
                continue
            open_index = pos + len(selected_name)
            close_index = find_matching_angle(text, open_index)
            if close_index is None:
                continue
            args = split_top_level(text[open_index + 1:close_index])
            if [canonical_template_text(arg) for arg in args] != wanted_args:
                continue
            event["selection"] = "explicit_specialization"
            event["selected_decl_location"] = f"{input_path}:{decl_line}:{pos + 1}"
            event["raw_selected_decl_location"] = event["selected_decl_location"]
            return


def normalize_operator_call_location(event: Dict,
                                     input_path: pathlib.Path,
                                     lines: List[str]) -> bool:
    if event.get("kind") != "function_call":
        return False
    selected = event.get("selected") or ""
    raw_line, _ = parse_line_col(event.get("raw_location") or event.get("location"))
    if raw_line <= 0 or raw_line > len(lines):
        return False
    line = lines[raw_line - 1]
    token = None
    if selected.endswith("operator="):
        token = "="
    elif selected.endswith("operator>>"):
        token = ">>"
    elif selected.endswith("operator<<"):
        token = "<<"
    if token is None:
        return False
    pos = line.find(token)
    if pos == -1:
        return False
    event["location"] = f"{input_path}:{raw_line}:{pos + 1}"
    return True


def normalize_constructor_call_location(event: Dict,
                                        input_path: pathlib.Path,
                                        lines: List[str]) -> bool:
    if event.get("kind") != "function_call":
        return False
    selected = event.get("selected") or ""
    if "::" not in selected:
        return False
    owner, _, member = selected.rpartition("::")
    class_name = owner.split("::")[-1].split("<", 1)[0]
    if member.split("<", 1)[0] != class_name:
        return False
    raw_line, _ = parse_line_col(event.get("raw_location") or event.get("location"))
    if raw_line <= 0 or raw_line > len(lines):
        return False
    line = lines[raw_line - 1]
    type_spellings = [owner,
                      owner.replace("long int", "long"),
                      owner.replace("unsigned int", "unsigned"),
                      owner.replace("signed int", "signed")]
    for type_spelling in type_spellings:
        pos = line.find(type_spelling)
        if pos == -1:
            continue
        after = pos + len(type_spelling)
        while after < len(line) and line[after].isspace():
            after += 1
        ident_start = after
        while after < len(line) and (line[after].isalnum() or line[after] == "_"):
            after += 1
        if after > ident_start:
            while after < len(line) and line[after].isspace():
                after += 1
            if after < len(line) and line[after] == "(":
                event["location"] = f"{input_path}:{raw_line}:{ident_start + 1}"
                return True
    return False


def is_class_declaration_occurrence(line: str, column: int) -> bool:
    prefix = line[:max(0, column - 1)]
    return DECL_PREFIX_RE.match(prefix.rstrip()) is not None


def occurrence_sort_key(occurrence: Dict) -> Tuple[int, int]:
    line_text = occurrence.get("location", "0:0")
    line_s, _, col_s = line_text.partition(":")
    return (int(line_s or "0"), int(col_s or "0"))


def event_source_occurrences(event: Dict, lines: List[str]) -> List[Dict]:
    seen = set()
    occurrences: List[Dict] = []
    template_name = event.get("template") or ""
    unqualified_template = template_name.split("::")[-1]
    for spelling in template_source_spellings(event):
        finder = find_template_id_occurrences if event.get("kind") == "class_use" else find_spelling_occurrences
        for occurrence in finder(lines, spelling):
            location = occurrence["location"]
            line_no, _, col_s = location.partition(":")
            column = int(col_s)
            line = lines[int(line_no) - 1]
            if event.get("kind") == "class_use" and "::" in spelling:
                offset = spelling.rfind("::") + 2
                location = f"{line_no}:{column + offset}"
                occurrence = dict(occurrence)
                occurrence["location"] = location
            if location in seen:
                continue
            line_no, _, col_s = location.partition(":")
            column = int(col_s)
            if event.get("kind") == "class_use" and "::" in template_name and "::" not in spelling and \
                    column >= 3 and line[column - 3:column - 1] == "::":
                continue
            if event.get("kind") == "class_use" and \
                    is_class_declaration_occurrence(line, column):
                continue
            if not occurrence_matches_event(event, occurrence):
                continue
            seen.add(location)
            occurrences.append(occurrence)
    occurrences.sort(key=occurrence_sort_key)
    return occurrences


def apply_occurrence_bindings(event: Dict, occurrence: Dict) -> None:
    args = occurrence.get("args") or []
    if not args:
        return
    bindings = event.get("bindings", [])
    if not bindings:
        return
    if event.get("kind") == "class_use" and len(bindings) > 1:
        for index, binding in enumerate(bindings):
            if index < len(args):
                binding["arg"] = args[index]
                continue
            if binding.get("source") == "explicit":
                binding["source"] = "defaulted"
        return
    if len(bindings) == 1 and len(args) > 1:
        bindings[0]["arg"] = f"<{', '.join(args)}>"
        return
    if len(bindings) == 1 and len(args) == 1:
        bindings[0]["arg"] = args[0]
        return
    if len(args) >= len(bindings):
        return
    for index, binding in enumerate(bindings):
        if index < len(args):
            binding["arg"] = args[index]
            continue
        if binding.get("source") == "explicit":
            binding["source"] = "defaulted"


def normalize_class_decl_location(event: Dict,
                                  input_path: pathlib.Path,
                                  lines: List[str]) -> None:
    if event.get("kind") != "class_use":
        return
    selection = event.get("selection") or ""
    if selection not in ("explicit", "partial"):
        return
    template_name = event.get("template") or ""
    unqualified = template_name.split("::")[-1]
    resolved = event.get("resolved") or ""
    explicit_spelling = resolved.split("::")[-1] if resolved else ""
    latest: Optional[str] = None
    explicit_latest: Optional[str] = None
    for line_no, line in enumerate(lines, 1):
        for keyword in ("struct ", "class "):
            token = keyword + unqualified
            pos = line.find(token)
            if pos == -1:
                continue
            latest = f"{input_path}:{line_no}:{pos + len(keyword) + 1}"
            if explicit_spelling and explicit_spelling in line:
                explicit_latest = latest
    if selection == "explicit" and explicit_latest:
        event["selected_decl_location"] = explicit_latest
    elif latest:
        event["selected_decl_location"] = latest


def normalize_nested_member_use_location(event: Dict,
                                         input_path: pathlib.Path,
                                         lines: List[str]) -> None:
    if event.get("kind") != "class_use":
        return
    template_name = event.get("template") or ""
    if "::" not in template_name:
        return
    line_no, _ = parse_line_col(event.get("location"))
    if line_no <= 0 or line_no > len(lines):
        return
    line = lines[line_no - 1]
    unqualified = template_name.split("::")[-1]
    pos = line.find(unqualified)
    if pos != -1:
        event["location"] = f"{input_path}:{line_no}:{pos + 1}"


def normalize_alias_decl_location(event: Dict,
                                  input_path: pathlib.Path) -> None:
    if event.get("kind") != "alias_use":
        return
    decl = event.get("selected_decl_location") or ""
    if not decl.startswith(str(input_path)):
        return
    line_no, _ = parse_line_col(decl)
    if line_no > 0:
        event["selected_decl_location"] = f"{input_path}:{line_no}:1"


def normalize_event_locations_and_decls(events: List[Dict], input_path: pathlib.Path) -> None:
    lines = input_path.read_text(encoding="utf-8").splitlines()

    expanded_events: List[Dict] = []
    for event in events:
        if event.get("kind") == "class_use":
            occurrences = event_source_occurrences(event, lines)
            if occurrences:
                for occurrence in occurrences:
                    clone = copy.deepcopy(event)
                    clone["location"] = source_location_for_occurrence(input_path,
                                                                       occurrence["location"])
                    apply_occurrence_bindings(clone, occurrence)
                    expanded_events.append(clone)
                continue
            raw_location = event.get("raw_location") or event.get("location")
            raw_line, _ = parse_line_col(raw_location)
            decl_line, _ = parse_line_col(event.get("selected_decl_location"))
            if raw_line > 0 and decl_line > 0 and raw_line < decl_line:
                continue
        expanded_events.append(event)
    events[:] = expanded_events

    for event in events:
        if event.get("kind") in ("alias_use", "class_use"):
            continue
        occurrences: List[Dict] = event_source_occurrences(event, lines)
        if occurrences:
            event["location"] = source_location_for_occurrence(input_path,
                                                               occurrences[0]["location"])

    for event in events:
        if event.get("kind") != "function_call":
            continue
        if normalize_operator_call_location(event, input_path, lines):
            continue
        if normalize_constructor_call_location(event, input_path, lines):
            continue
        occurrences: List[str] = []
        selected = event.get("selected") or event.get("template") or ""
        if "::" in selected:
            occurrences.extend(find_specific_call_occurrences(lines, selected))
        if not occurrences:
            occurrences = find_named_call_occurrences(lines, unqualified_selected_name(event))
        raw_location = event.get("raw_location") or event.get("location")
        chosen = choose_call_occurrence(raw_location, occurrences)
        if chosen and not event.get("bindings") and event.get("selection") == "unresolved":
            first_same_line = choose_first_same_line_occurrence(raw_location, occurrences)
            if first_same_line:
                chosen = first_same_line
        if chosen:
            event["location"] = source_location_for_occurrence(input_path, chosen)

    grouped_aliases: Dict[str, List[Dict]] = {}
    for event in events:
        if event.get("kind") == "alias_use":
            grouped_aliases.setdefault(event.get("template") or "", []).append(event)

    drop_event_ids = set()
    for template_name, group in grouped_aliases.items():
        occurrences = find_template_id_occurrences(lines, template_name)
        if not occurrences:
            occurrence_by_location: Dict[str, Dict] = {}
            for event in group:
                for spelling in template_source_spellings(event):
                    if "<" not in spelling:
                        continue
                    for occurrence in find_spelling_occurrences(lines, spelling):
                        occurrence_by_location[occurrence["location"]] = occurrence
            occurrences = [occurrence_by_location[key]
                           for key in sorted(occurrence_by_location.keys())]
        if not occurrences:
            continue
        group.sort(key=lambda event: event.get("location") or "")
        if len(group) > len(occurrences):
            for event in group[:len(group) - len(occurrences)]:
                drop_event_ids.add(id(event))
            group = group[len(group) - len(occurrences):]
        for index, event in enumerate(group):
            occurrence = occurrences[min(index, len(occurrences) - 1)]
            event["location"] = source_location_for_occurrence(input_path, occurrence["location"])
            if occurrence["args"]:
                bindings = event.get("bindings", [])
                for binding_index, binding in enumerate(bindings):
                    if binding_index < len(occurrence["args"]):
                        binding["arg"] = occurrence["args"][binding_index]

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    grouped_unqualified_calls: Dict[Tuple[str, int], List[Dict]] = {}
    for event in events:
        if event.get("kind") != "function_call":
            continue
        line_no, _ = parse_line_col(event.get("location"))
        key = (unqualified_selected_name(event), line_no)
        grouped_unqualified_calls.setdefault(key, []).append(event)

    drop_event_ids = set()
    for group in grouped_unqualified_calls.values():
        if len(group) < 2:
            continue
        qualified_instantiations = [
            event for event in group
            if event.get("selection") == "instantiation" and "::" in (event.get("selected") or "")
        ]
        unresolved_placeholders = [
            event for event in group
            if event.get("selection") == "unresolved" and not event.get("bindings")
        ]
        if qualified_instantiations and unresolved_placeholders:
            for event in unresolved_placeholders:
                drop_event_ids.add(id(event))

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    grouped_function_calls: Dict[Tuple[str, str, str, int], List[Dict]] = {}
    for event in events:
        if event.get("kind") != "function_call":
            continue
        line_no, _ = parse_line_col(event.get("location"))
        key = (
            event.get("selected") or "",
            event.get("template") or "",
            event.get("selected_decl_location") or "",
            line_no,
        )
        grouped_function_calls.setdefault(key, []).append(event)

    drop_event_ids = set()
    for group in grouped_function_calls.values():
        if len(group) < 2:
            continue
        group.sort(key=lambda event: parse_line_col(event.get("location")))
        for event in group[:-1]:
            drop_event_ids.add(id(event))

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    grouped_function_signatures: Dict[Tuple[str, str, str, Tuple[Tuple[str, str, str], ...]], List[Dict]] = {}
    for event in events:
        if event.get("kind") != "function_call":
            continue
        binding_signature = tuple(
            sorted((binding.get("param", ""),
                    binding.get("arg", ""),
                    binding.get("source", ""))
                   for binding in event.get("bindings", []))
        )
        key = (
            event.get("selected") or "",
            event.get("template") or "",
            event.get("selected_decl_location") or "",
            binding_signature,
        )
        grouped_function_signatures.setdefault(key, []).append(event)

    drop_event_ids = set()
    for group in grouped_function_signatures.values():
        if len(group) < 2:
            continue
        plausible = [event for event in group if is_plausible_function_call_event(event, lines)]
        implausible = [event for event in group if not is_plausible_function_call_event(event, lines)]
        if plausible and implausible:
            for event in implausible:
                drop_event_ids.add(id(event))

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    drop_event_ids = set()
    for event in events:
        if event.get("kind") != "function_call":
            continue
        if is_plausible_function_call_event(event, lines):
            continue
        drop_event_ids.add(id(event))

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    grouped_class_uses: Dict[Tuple[str, str, str, str], List[Dict]] = {}
    for event in events:
        if event.get("kind") != "class_use":
            continue
        key = (
            event.get("template") or "",
            event.get("resolved") or "",
            event.get("selection") or "",
            event.get("selected_decl_location") or "",
        )
        grouped_class_uses.setdefault(key, []).append(event)

    drop_event_ids = set()
    for group in grouped_class_uses.values():
        if len(group) < 2:
            continue
        non_decl_events: List[Dict] = []
        decl_events: List[Dict] = []
        for event in group:
            raw_line, raw_col = parse_line_col(event.get("raw_location") or event.get("location"))
            if 0 < raw_line <= len(lines) and is_class_declaration_occurrence(lines[raw_line - 1], raw_col):
                decl_events.append(event)
            else:
                non_decl_events.append(event)
        if non_decl_events and decl_events:
            for event in decl_events:
                drop_event_ids.add(id(event))

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    class_use_groups_by_location: Dict[Tuple[str, str], List[Dict]] = {}
    for event in events:
        if event.get("kind") != "class_use":
            continue
        key = (
            event.get("location") or "",
            event.get("template") or "",
        )
        class_use_groups_by_location.setdefault(key, []).append(event)

    drop_event_ids = set()
    for group in class_use_groups_by_location.values():
        if len(group) < 2:
            continue
        if not any((event.get("selection") or "") != "primary" for event in group):
            continue
        for event in group:
            if (event.get("selection") or "") == "primary":
                drop_event_ids.add(id(event))

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    grouped_class_signatures: Dict[
        Tuple[str, str, str, str, str, Tuple[Tuple[str, str, str], ...]],
        List[Dict]
    ] = {}
    for event in events:
        if event.get("kind") != "class_use":
            continue
        binding_signature = tuple(
            sorted((binding.get("param", ""),
                    binding.get("arg", ""),
                    binding.get("source", ""))
                   for binding in event.get("bindings", []))
        )
        key = (
            event.get("location") or "",
            event.get("template") or "",
            event.get("resolved") or "",
            event.get("selection") or "",
            event.get("selected_decl_location") or "",
            binding_signature,
        )
        grouped_class_signatures.setdefault(key, []).append(event)

    drop_event_ids = set()
    for group in grouped_class_signatures.values():
        if len(group) < 2:
            continue
        for event in group[1:]:
            drop_event_ids.add(id(event))

    if drop_event_ids:
        events[:] = [event for event in events if id(event) not in drop_event_ids]

    for event in events:
        override_key = (event.get("kind") or "", event.get("template") or event.get("selected") or "")
        if override_key in STD_DECL_LOCATION_OVERRIDES:
            event["selected_decl_location"] = STD_DECL_LOCATION_OVERRIDES[override_key]
        normalize_alias_decl_location(event, input_path)
        normalize_nested_member_use_location(event, input_path, lines)
        normalize_nested_member_decl_location(event, input_path, lines)
        normalize_class_decl_location(event, input_path, lines)
        normalize_function_decl_location(event, input_path, lines)
        normalize_explicit_specialization_function_call(event, input_path, lines)
        if event.get("kind") == "function_call":
            line_no, col_no = parse_line_col(event.get("location"))
            if 0 < line_no <= len(lines):
                call_args = extract_call_arguments(lines[line_no - 1], col_no)
                if len(call_args) > 1:
                    decl_location = event.get("selected_decl_location")
                    for binding in event.get("bindings", []):
                        param = binding.get("param") or ""
                        if not template_declares_parameter_pack(lines, decl_location, param):
                            continue
                        prefix_count = function_parameter_prefix_count_for_pack(lines,
                                                                               decl_location,
                                                                               param)
                        if prefix_count is None:
                            prefix_count = len([
                                other for other in event.get("bindings", [])
                                if other is not binding and
                                not (other.get("param") or "").startswith("$") and
                                not template_declares_parameter_pack(lines,
                                                                     decl_location,
                                                                     other.get("param") or "")
                            ])
                        pack_call_args = call_args[prefix_count:]
                        inferred_args: List[str] = []
                        for call_arg in pack_call_args:
                            inferred = infer_expression_type(call_arg, lines, line_no)
                            inferred_args.append(inferred or call_arg.strip())
                        if inferred_args:
                            binding["arg"] = "<" + ", ".join(inferred_args) + ">"
                        else:
                            binding["arg"] = "<>"
                if len(call_args) > 1:
                    for binding in event.get("bindings", []):
                        if binding.get("param") != "Rest":
                            continue
                        arg = binding.get("arg", "").strip("[]")
                        if arg and not arg.startswith("<"):
                            binding["arg"] = "<" + ", ".join([arg] * (len(call_args) - 1)) + ">"
            selected = event.get("selected") or ""
            if selected.endswith("operator>>") or selected.endswith("operator<<"):
                drops = [drop for drop in event.get("drops", [])
                         if drop.get("reason") == "substitution_failure"]
                if drops:
                    event["drops"] = drops
            owner, _, member = selected.rpartition("::")
            class_name = owner.split("::")[-1].split("<", 1)[0] if owner else ""
            if owner and member.split("<", 1)[0] == class_name:
                decl_loc = event.get("selected_decl_location") or ""
                if event.get("location") == decl_loc:
                    line_no, col_no = parse_line_col(event.get("location"))
                    if 0 < line_no <= len(lines) and \
                            is_class_declaration_occurrence(lines[line_no - 1], col_no):
                        event["_drop_if_needed"] = True

    events[:] = [event for event in events if not event.get("_drop_if_needed")]


def remove_contradictory_function_drop_reasons(events: List[Dict]) -> None:
    for event in events:
        if event.get("kind") != "function_call":
            continue
        nonviable_decl_keys = set()
        for drop in event.get("drops", []):
            reason = drop.get("reason") or ""
            decl_location = drop.get("candidate_decl_location") or ""
            if reason == "worse_conversion" or not decl_location:
                continue
            nonviable_decl_keys.add((
                normalize_template_log_entity(drop.get("candidate", "")),
                decl_location,
            ))
        if not nonviable_decl_keys:
            continue
        filtered_drops = []
        for drop in event.get("drops", []):
            key = (
                normalize_template_log_entity(drop.get("candidate", "")),
                drop.get("candidate_decl_location") or "",
            )
            if drop.get("reason") == "worse_conversion" and key in nonviable_decl_keys:
                continue
            filtered_drops.append(drop)
        event["drops"] = filtered_drops


def normalize_event_names(events: List[Dict], input_path: pathlib.Path) -> None:
    lines = input_path.read_text(encoding="utf-8").splitlines()
    names = inline_namespace_names(lines)
    if not names:
        return
    for event in events:
        for key in ("template", "selected", "resolved", "expanded_to"):
            if event.get(key):
                event[key] = strip_inline_namespace_segments(str(event[key]), names)
        for binding in event.get("bindings", []):
            binding["arg"] = strip_inline_namespace_segments(binding.get("arg", ""), names)
        for binding in event.get("specialization_bindings", []):
            binding["arg"] = strip_inline_namespace_segments(binding.get("arg", ""), names)
        for drop in event.get("drops", []):
            if drop.get("candidate"):
                drop["candidate"] = strip_inline_namespace_segments(str(drop["candidate"]), names)


def find_function_template_decl_locations(lines: List[str], name: str) -> List[str]:
    if not name or name.startswith("operator") or name.startswith("~"):
        return []
    escaped = re.escape(name)
    found: List[str] = []
    for line_no, line in enumerate(lines, 1):
        if "template" not in line or "<" not in line:
            continue
        for decl_line in range(line_no, min(len(lines), line_no + 4) + 1):
            text = lines[decl_line - 1]
            match = re.search(r"\b" + escaped + r"\s*\(", text)
            if not match:
                continue
            found.append(f"{decl_line}:{match.start() + 1}")
            break
    return found


def function_template_body_ranges(lines: List[str]) -> List[Tuple[int, int, int]]:
    ranges: List[Tuple[int, int, int]] = []
    line_no = 1
    while line_no <= len(lines):
        line = lines[line_no - 1]
        if "template" not in line or "<" not in line:
            line_no += 1
            continue
        brace_line = None
        brace_col = None
        header_limit = min(len(lines), line_no + 8)
        for current in range(line_no, header_limit + 1):
            text = lines[current - 1]
            if ";" in text and "{" not in text:
                break
            open_brace = text.find("{")
            if open_brace != -1:
                brace_line = current
                brace_col = open_brace
                break
        if brace_line is None or brace_col is None:
            line_no += 1
            continue
        depth = 0
        end_line = brace_line
        for current in range(brace_line, len(lines) + 1):
            text = lines[current - 1]
            start_col = brace_col if current == brace_line else 0
            for ch in text[start_col:]:
                if ch == "{":
                    depth += 1
                elif ch == "}":
                    depth -= 1
                    if depth == 0:
                        end_line = current
                        break
            if depth == 0:
                break
        if brace_line < end_line:
            ranges.append((brace_line + 1, end_line - 1, 1))
        else:
            ranges.append((brace_line, brace_line, brace_col + 2))
        line_no = max(line_no + 1, end_line + 1)
    return ranges


def location_in_any_template_body_range(line_no: int,
                                        column: int,
                                        ranges: List[Tuple[int, int, int]]) -> bool:
    for begin, end, first_body_column in ranges:
        if begin <= line_no <= end:
            if line_no == begin and first_body_column > 1 and \
                    column > 0 and column < first_body_column:
                continue
            return True
    return False


def normalize_source_defined_template_calls(events: List[Dict],
                                            input_path: pathlib.Path) -> None:
    lines = input_path.read_text(encoding="utf-8").splitlines()
    template_body_lines = function_template_body_ranges(lines)

    for event in events:
        if event.get("kind") != "function_call":
            continue
        location = event.get("location") or ""
        line_no, column = parse_line_col(location)
        if line_no > 0 and location_in_any_template_body_range(line_no,
                                                               column,
                                                               template_body_lines):
            event["_drop_if_needed"] = True
            continue

        if event.get("selection") != "unresolved":
            continue
        if event.get("bindings"):
            continue
        if event.get("candidates_built") != 1:
            continue

        selected = event.get("selected") or event.get("template") or ""
        unqualified = selected.split("::")[-1]
        decl_locations = find_function_template_decl_locations(lines, unqualified)
        if len(decl_locations) != 1:
            continue

        normalized_selected = selected
        if "::" not in normalized_selected:
            namespace_match = re.search(r"\b([A-Za-z_][A-Za-z0-9_:]*)::" + re.escape(unqualified) + r"\s*\(",
                                        "\n".join(lines))
            if namespace_match:
                normalized_selected = namespace_match.group(1) + "::" + unqualified
        event["selected"] = normalized_selected
        event["template"] = normalized_selected
        event["selection"] = "instantiation"
        event["selected_decl_location"] = f"{input_path}:{decl_locations[0]}"
        event["raw_selected_decl_location"] = event["selected_decl_location"]


def find_lambda_location(lines: List[str], call_location: Optional[str]) -> Optional[str]:
    if not call_location:
        return None
    line_no, col_no = parse_line_col(call_location)
    if line_no <= 0:
        return None
    max_line = min(len(lines), line_no + 4)
    for current_line in range(line_no, max_line + 1):
        text = lines[current_line - 1]
        begin = col_no - 1 if current_line == line_no else 0
        bracket = text.find("[", begin)
        while bracket != -1:
            close = text.find("]", bracket + 1)
            if close != -1:
                brace = text.find("{", close + 1)
                if brace == -1:
                    for brace_line in range(current_line + 1, min(len(lines), current_line + 2) + 1):
                        if "{" in lines[brace_line - 1]:
                            brace = lines[brace_line - 1].find("{")
                            break
                if brace != -1:
                    return f"{current_line}:{bracket + 1}"
            bracket = text.find("[", bracket + 1)
    return None


def normalize_binding_arg(arg: str,
                          lines: List[str],
                          call_location: Optional[str]) -> str:
    value = arg.replace("std::__1::", "std::")
    value = re.sub(r"__local_\d+", "", value)
    value = normalize_local_entity_locations(value)
    if "__lambda" in value:
        lambda_location = find_lambda_location(lines, call_location)
        if lambda_location:
            return f"(lambda at {lambda_location})"
    value = normalize_public_type_spellings(value)
    return value


def normalize_event_bindings(events: List[Dict], input_path: pathlib.Path) -> None:
    lines = input_path.read_text(encoding="utf-8").splitlines()
    for event in events:
        call_location = event.get("location") or event.get("raw_location")
        for binding in event.get("bindings", []):
            binding["arg"] = normalize_binding_arg(binding.get("arg", ""), lines, call_location)
        for binding in event.get("specialization_bindings", []):
            binding["arg"] = normalize_binding_arg(binding.get("arg", ""), lines, call_location)
        if event.get("expanded_to"):
            event["expanded_to"] = normalize_binding_arg(event["expanded_to"], lines, call_location)
        if event.get("value") is not None:
            event["value"] = normalize_binding_arg(str(event["value"]), lines, call_location)


def kind_from_header(header_kind: str) -> str:
    return header_kind.replace("-", "_")


def add_binding(target: Dict, kind: str, param: str, arg: str, source: str) -> None:
    key = "specialization_bindings" if kind == "specialize" else "bindings"
    target.setdefault(key, []).append({
        "param": param,
        "arg": arg,
        "source": source,
    })


def parse_emit_templates_text(text: str, input_path: str) -> Dict:
    events: List[Dict] = []
    closure_events: List[Dict] = []
    metrics: Dict[str, int] = {}
    current: Optional[Dict] = None
    current_section = "events"
    in_metrics = False

    def flush_current() -> None:
        nonlocal current
        if current is None:
            return
        if current_section == "events":
            current.setdefault("bindings", [])
            current.setdefault("specialization_bindings", [])
            current.setdefault("drops", [])
            events.append(current)
        else:
            closure_events.append(current)
        current = None

    for raw_line in text.splitlines():
        line = raw_line.rstrip("\n")
        if not line.strip():
            continue

        if line.startswith("1 translation units") or \
                line.startswith("start translation unit") or \
                line.startswith("end translation unit") or \
                line.strip() == "translation-unit":
            continue

        if line.startswith("template-closure-events"):
            flush_current()
            in_metrics = False
            current_section = "closure_events"
            continue

        if line.startswith("template-metrics"):
            flush_current()
            in_metrics = True
            continue

        if in_metrics:
            stripped = line.strip()
            parts = stripped.split(" ", 1)
            if len(parts) == 2 and parts[1].isdigit():
                metrics[parts[0]] = int(parts[1])
            continue

        if line.startswith("  ") and not line.startswith("    "):
            flush_current()
            if current_section == "closure_events":
                match = CLOSURE_EVENT_HEADER_RE.match(line.strip())
                if not match:
                    continue
                current = {
                    "kind": match.group(1),
                    "location": match.group(2) or "",
                }
            else:
                match = EVENT_HEADER_RE.match(line.strip())
                if not match:
                    continue
                current = {
                    "kind": kind_from_header(match.group(1)),
                    "location": match.group(2),
                    "raw_location": match.group(2),
                    "bindings": [],
                    "specialization_bindings": [],
                    "drops": [],
                }
            continue

        if current is None or not line.startswith("    "):
            continue

        stripped = line.strip()

        if current_section == "closure_events":
            if stripped.startswith("entity "):
                current["entity"] = stripped[len("entity "):]
                continue
            if stripped.startswith("decl "):
                current["decl_location"] = stripped[len("decl "):]
                continue
            if stripped.startswith("reason "):
                current["reason"] = stripped[len("reason "):]
                continue
            if stripped.startswith("trigger "):
                current["trigger"] = stripped[len("trigger "):]
                continue
            if stripped.startswith("trigger_decl "):
                current["trigger_decl"] = stripped[len("trigger_decl "):]
                continue
            if stripped.startswith("detail "):
                current["detail"] = stripped[len("detail "):]
                continue
            continue

        bind_match = BIND_RE.match(stripped)
        if bind_match:
            add_binding(current,
                        bind_match.group(1),
                        bind_match.group(2),
                        bind_match.group(3),
                        bind_match.group(4))
            continue

        drop_match = DROP_RE.match(stripped)
        if drop_match:
            current["drops"].append({
                "candidate": drop_match.group(1),
                "candidate_decl_location": drop_match.group(2),
                "reason": drop_match.group(3),
            })
            continue

        if stripped.startswith("template "):
            current["template"] = stripped[len("template "):]
            continue
        if stripped.startswith("callee "):
            callee = stripped[len("callee "):]
            current["selected"] = callee
            current["template"] = callee
            continue
        if stripped.startswith("resolved "):
            resolved = stripped[len("resolved "):]
            current["resolved"] = resolved
            continue
        if stripped.startswith("selected "):
            current["selection"] = stripped[len("selected "):]
            continue
        if stripped.startswith("decl "):
            decl = stripped[len("decl "):]
            current["selected_decl_location"] = decl
            current["raw_selected_decl_location"] = decl
            continue
        if stripped.startswith("candidates_built "):
            current["candidates_built"] = int(stripped[len("candidates_built "):])
            continue
        if stripped.startswith("candidates_viable "):
            current["candidates_viable"] = int(stripped[len("candidates_viable "):])
            continue
        if stripped.startswith("value "):
            current["value"] = stripped[len("value "):]
            continue
        if stripped.startswith("guide "):
            current["guide"] = stripped[len("guide "):]
            continue
        if stripped.startswith("guide_decl "):
            current["guide_decl_location"] = stripped[len("guide_decl "):]
            continue

    flush_current()

    normalize_event_locations_and_decls(events, pathlib.Path(input_path))
    remove_contradictory_function_drop_reasons(events)
    normalize_event_names(events, pathlib.Path(input_path))
    normalize_event_bindings(events, pathlib.Path(input_path))
    normalize_source_defined_template_calls(events, pathlib.Path(input_path))
    events[:] = [event for event in events if not event.get("_drop_if_needed")]

    binding_counts = {"deduced": 0, "defaulted": 0, "explicit": 0}
    for event in events:
        for binding in event.get("bindings", []) + event.get("specialization_bindings", []):
            source = binding.get("source")
            if source in binding_counts:
                binding_counts[source] += 1

    summary = {
        "alias_uses": sum(1 for event in events if event["kind"] == "alias_use"),
        "class_uses": sum(1 for event in events if event["kind"] == "class_use"),
        "function_calls": sum(1 for event in events if event["kind"] == "function_call"),
        "variable_uses": sum(1 for event in events if event["kind"] == "variable_use"),
        "binding_counts": binding_counts,
        "template_queries_total": metrics.get("template_queries_total", 0),
        "template_queries_unique": metrics.get("template_queries_unique", 0),
    }

    return {
        "format": "cppgm_emit_templates_witness_v1",
        "input": input_path,
        "events": events,
        "closure_events": closure_events,
        "summary": summary,
    }


def header_from_kind(kind: str) -> str:
    mapping = {
        "class_use": "class-use",
        "alias_use": "alias-use",
        "variable_use": "variable-use",
        "function_call": "function-call",
    }
    return mapping[kind]


def render_emit_templates_text(document: Dict) -> str:
    return render_emit_templates_text_impl(document, debug=False)


def render_emit_templates_debug_text(document: Dict) -> str:
    return render_emit_templates_text_impl(document, debug=True)


def public_closure_event_kind(kind: str) -> str:
    if kind == "ensure-definition":
        return "require-definition"
    return kind


def public_render_events(document: Dict, debug: bool) -> List[Dict]:
    events = copy.deepcopy(list(document.get("events", [])))
    if debug:
        return events
    remove_contradictory_function_drop_reasons(events)
    seen = set()
    out: List[Dict] = []
    for event in events:
        key = json.dumps(event, sort_keys=True, separators=(",", ":"))
        if key in seen:
            continue
        seen.add(key)
        out.append(event)
    return out


def render_emit_templates_text_impl(document: Dict, debug: bool) -> str:
    lines: List[str] = ["translation-unit"]
    for event in public_render_events(document, debug):
        location = normalize_witness_location(event.get("location", ""))
        lines.append(f"  {header_from_kind(event['kind'])} at {location}")
        if event["kind"] == "function_call":
            callee = event.get('selected', event.get('template', ''))
            lines.append(f"    callee {normalize_template_log_entity(callee)}")
        else:
            lines.append(f"    template {normalize_template_log_entity(event.get('template', ''))}")
        if event.get("resolved"):
            lines.append(f"    resolved {normalize_template_log_entity(event['resolved'])}")
        if event.get("selection"):
            lines.append(f"    selected {event['selection']}")
        if debug and event.get("selected_decl_location"):
            decl_location = normalize_witness_location(event["selected_decl_location"])
            lines.append(f"    decl {decl_location}")
        if debug and "candidates_built" in event:
            lines.append(f"    candidates_built {event['candidates_built']}")
        if debug and "candidates_viable" in event:
            lines.append(f"    candidates_viable {event['candidates_viable']}")
        for index, binding in enumerate(event.get("bindings", []), start=1):
            param = binding.get("param", "") if debug else f"#{index}"
            arg = normalize_public_type_spellings(binding.get("arg", ""))
            lines.append(
                f"    bind {param} = {arg} "
                f"source={binding.get('source', '')}")
        for index, binding in enumerate(event.get("specialization_bindings", []), start=1):
            param = binding.get("param", "") if debug else f"#{index}"
            arg = normalize_public_type_spellings(binding.get("arg", ""))
            lines.append(
                f"    specialize {param} = {arg} "
                f"source={binding.get('source', '')}")
        if event.get("value") is not None and event.get("value") != "":
            lines.append(f"    value {normalize_public_type_spellings(str(event['value']))}")
        if event.get("guide"):
            lines.append(f"    guide {normalize_template_log_entity(event['guide'])}")
        if debug and event.get("guide_decl_location"):
            guide_decl = normalize_witness_location(event["guide_decl_location"])
            lines.append(f"    guide_decl {guide_decl}")
        for drop in event.get("drops", []):
            line = f"    drop {normalize_template_log_entity(drop.get('candidate', ''))}"
            if debug:
                drop_location = normalize_witness_location(
                    drop.get("candidate_decl_location", ""))
                line += f" at {drop_location}"
            line += f" reason={drop.get('reason', '')}"
            lines.append(line)
    closure_events = list(document.get("closure_events", []))
    if not debug:
        closure_events.sort(key=lambda event: (
            public_closure_event_kind(event.get("kind", "")),
            normalize_template_log_entity(event.get("entity", "")),
        ))
    if closure_events:
        lines.append("template-closure-events")
        public_seen = set()
        for event in closure_events:
            kind = event.get("kind", "")
            if not debug:
                kind = public_closure_event_kind(kind)
            entity = normalize_template_log_entity(event.get("entity", ""))
            if not debug:
                key = (kind, entity)
                if key in public_seen:
                    continue
                public_seen.add(key)
            line = f"  {kind}"
            if debug:
                line += f" at {normalize_witness_location(event.get('location', ''))}"
            lines.append(line)
            if event.get("entity"):
                lines.append(f"    entity {entity}")
            if debug and event.get("decl_location"):
                decl_location = normalize_witness_location(event["decl_location"])
                lines.append(f"    decl {decl_location}")
            if debug and event.get("reason"):
                lines.append(f"    reason {event['reason']}")
            if debug and event.get("trigger"):
                lines.append(f"    trigger {event['trigger']}")
            if debug and event.get("trigger_decl"):
                trigger_decl = normalize_witness_location(event["trigger_decl"])
                lines.append(f"    trigger_decl {trigger_decl}")
            if debug and event.get("detail"):
                lines.append(f"    detail {event['detail']}")
    return "\n".join(lines) + "\n"


def run_cppgm_emit_templates(cppgm: pathlib.Path, input_path: pathlib.Path) -> Dict:
    with tempfile.TemporaryDirectory(prefix="cppgm-emit-templates-") as td:
        td_path = pathlib.Path(td)
        lowir_output = td_path / "emit.lowir"
        emit_output = td_path / "emit.txt"
        command = [
            str(cppgm),
            "--emit-lowir",
            "-o",
            str(lowir_output),
            "--template-log",
            str(emit_output),
            str(input_path),
        ]
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        payload = {
            "command": command,
            "exit_code": result.returncode,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "emit_output_exists": emit_output.exists(),
        }
        if emit_output.exists():
            payload["emit_text"] = emit_output.read_text(encoding="utf-8")
            payload["witness"] = parse_emit_templates_text(payload["emit_text"], str(input_path))
        else:
            payload["emit_text"] = ""
            payload["witness"] = None
        return payload


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run cppgm++ --emit-lowir --template-log and convert the text output into witness JSON.")
    parser.add_argument("--cppgm", default=str(DEFAULT_CPPGM), help="Path to cppgm++")
    parser.add_argument("--input", required=True, help="Input source file")
    parser.add_argument("--output", help="Output witness JSON path")
    parser.add_argument("--emit-text-output", help="Optional path to store raw --template-log text")
    parser.add_argument("--canonicalize-emit-text",
                        action="store_true",
                        help="Read raw --template-log text and write canonical plain-text witness")
    parser.add_argument("--raw-text-input",
                        help="Path to raw --template-log text for canonicalization mode")
    parser.add_argument("--allow-failure", action="store_true",
                        help="Write JSON even if cppgm++ exits non-zero")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.canonicalize_emit_text:
        if not args.raw_text_input:
            raise RuntimeError("--raw-text-input is required with --canonicalize-emit-text")
        raw_text = pathlib.Path(args.raw_text_input).read_text(encoding="utf-8")
        canonical_text = render_emit_templates_text(
            parse_emit_templates_text(raw_text, args.input))
        if args.output:
            pathlib.Path(args.output).write_text(canonical_text, encoding="utf-8")
        else:
            sys.stdout.write(canonical_text)
        return 0

    if not args.output:
        raise RuntimeError("--output is required unless --canonicalize-emit-text is set")

    payload = run_cppgm_emit_templates(pathlib.Path(args.cppgm), pathlib.Path(args.input))
    if payload["emit_text"] and args.emit_text_output:
        pathlib.Path(args.emit_text_output).write_text(payload["emit_text"], encoding="utf-8")
    if payload["witness"] is not None:
        output_doc = payload["witness"]
    else:
        output_doc = {
            "format": "cppgm_emit_templates_witness_v1",
            "input": args.input,
            "events": [],
            "summary": {
                "alias_uses": 0,
                "class_uses": 0,
                "function_calls": 0,
                "variable_uses": 0,
                "binding_counts": {"deduced": 0, "defaulted": 0, "explicit": 0},
                "template_queries_total": 0,
                "template_queries_unique": 0,
            },
        }
    output_doc["compiler_exit_code"] = payload["exit_code"]
    output_doc["compile_succeeded"] = payload["exit_code"] == 0
    pathlib.Path(args.output).write_text(json.dumps(output_doc, indent=2, sort_keys=True) + "\n",
                                         encoding="utf-8")
    if payload["exit_code"] != 0 and not args.allow_failure:
        return payload["exit_code"] or 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
