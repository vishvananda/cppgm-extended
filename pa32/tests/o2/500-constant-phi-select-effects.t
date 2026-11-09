function @select(%condition : u8, %port : ptr) -> u32 {
  block ^entry:
    branch %condition, ^yes, ^no

  block ^yes:
    %yes_value = load volatile u32 %port
    jump ^merge

  block ^no:
    %discarded = load volatile u32 %port
    jump ^merge

  block ^merge:
    %tag = phi u8 [^yes: 1, ^no: 0]
    %payload = phi u32 [^yes: %yes_value, ^no: %discarded]
    %result = select u32 %tag, %payload, 3
    return u32 %result
}

function @unknown(%condition : u8, %tag_input : u8, %input : u32) -> u32 {
  block ^entry:
    branch %condition, ^yes, ^no

  block ^yes:
    jump ^merge

  block ^no:
    jump ^merge

  block ^merge:
    %tag = phi u8 [^yes: 1, ^no: %tag_input]
    %payload = phi u32 [^yes: %input, ^no: 17]
    %result = select u32 %tag, %payload, 3
    return u32 %result
}

function @late_value(%condition : u8, %input : u32) -> u32 {
  block ^entry:
    branch %condition, ^yes, ^no

  block ^yes:
    jump ^merge

  block ^no:
    jump ^merge

  block ^merge:
    %tag = phi u8 [^yes: 1, ^no: 0]
    %computed = binary add u32 %input, 7
    %result = select u32 %tag, %computed, 3
    return u32 %result
}

function @truncated(%condition : u8, %input : u32) -> u32 {
  block ^entry:
    branch %condition, ^yes, ^no

  block ^yes:
    jump ^merge

  block ^no:
    jump ^merge

  block ^merge:
    %tag = phi u8 [^yes: 256, ^no: 255]
    %payload = phi u32 [^yes: %input, ^no: 17]
    %result = select u32 %tag, %payload, 3
    return u32 %result
}
