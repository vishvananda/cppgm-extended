import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
AUDIT = ROOT / "scripts" / "cppgm_file_audit.pl"


class FileAuditFunctions(unittest.TestCase):
    def audit(self, member_body):
        # A short initializer-list constructor used to remain in the signature
        # buffer and turn the following entire class into a function named tail.
        source = """struct Step
{
    int tail;
    int temporary;
    int type;
    int destructor;
    int destination;
    Step(int previous, int action,
        int object_type, int binding,
        int address)
        : tail(previous), temporary(action), type(object_type),
          destructor(binding), destination(address) {}
};
template <class Derived>
class Prefix
{
public:
    Prefix() : root(0), active(false) {}
    void member()
    {
BODY
    }
METHODS
private:
    int root;
    bool active;
};
"""
        methods = "\n".join(
            f"    int value{i}() const\n    {{\n        return {i};\n    }}\n"
            for i in range(50)
        )
        source = source.replace("BODY", member_body).replace("METHODS", methods)
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "prefix.cpp").write_text(source)
            return subprocess.run(
                ["perl", str(AUDIT), "--root", str(root), "--paths", "prefix.cpp"],
                capture_output=True,
                text=True,
                check=False,
            )

    def test_class_size_is_not_function_size(self):
        result = self.audit("        ++root;")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("[function-size]", result.stdout)

    def test_oversized_member_still_fails(self):
        result = self.audit("\n".join("        ++root;" for _ in range(241)))
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("[fatal][function-size]", result.stdout)
        self.assertIn("member is 243 lines", result.stdout)


if __name__ == "__main__":
    unittest.main()
