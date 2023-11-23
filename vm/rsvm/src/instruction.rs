use nom::types::CompleteStr;

#[repr(u8)]
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Opcode {
    LOAD = 0,

    ADD = 10,
    SUB = 11,
    MUL = 12,
    DIV = 13,

    EQ = 20,
    NEQ = 21,
    GT = 22,
    LT = 23,
    GTE = 24,
    LTE = 25,

    JEQ = 26,
    JNEQ = 27,

    JMPB = 79,
    JMP = 80,
    JMPF = 81,

    NOP = 98,
    HLT = 99,

    IGL, // illegal
}

pub struct Instruction {
    opcode: Opcode,
}

impl Into<u8> for Opcode {
    fn into(self) -> u8 {
        self as u8
    }
}

impl From<u8> for Opcode {
    fn from(value: u8) -> Self {
        match value {
            0 => Self::LOAD,

            10 => Self::ADD,
            11 => Self::SUB,
            12 => Self::MUL,
            13 => Self::DIV,

            20 => Self::EQ,
            21 => Self::NEQ,
            22 => Self::GT,
            23 => Self::LT,
            24 => Self::GTE,
            25 => Self::LTE,

            26 => Self::JEQ,
            27 => Self::JNEQ,

            79 => Self::JMPB,
            80 => Self::JMP,
            81 => Self::JMPF,

            99 => Self::HLT,

            _ => Self::IGL,
        }
    }
}

impl<'a> From<CompleteStr<'a>> for Opcode {
    fn from(value: CompleteStr<'a>) -> Self {
        match value {
            CompleteStr("load") => Opcode::LOAD,

            CompleteStr("add") => Opcode::ADD,
            CompleteStr("sub") => Opcode::SUB,
            CompleteStr("mul") => Opcode::MUL,
            CompleteStr("div") => Opcode::DIV,

            CompleteStr("eq") => Opcode::EQ,
            CompleteStr("neq") => Opcode::NEQ,
            CompleteStr("gte") => Opcode::GTE,
            CompleteStr("gt") => Opcode::GT,
            CompleteStr("lte") => Opcode::LTE,
            CompleteStr("lt") => Opcode::LT,

            CompleteStr("hlt") => Opcode::HLT,
            CompleteStr("jmp") => Opcode::JMP,
            CompleteStr("jmpf") => Opcode::JMPF,
            CompleteStr("jmpb") => Opcode::JMPB,

            CompleteStr("jeq") => Opcode::JEQ,
            CompleteStr("jneq") => Opcode::JNEQ,
            CompleteStr("nop") => Opcode::NOP,

            _ => Opcode::IGL,
        }
    }
}

impl Instruction {
    pub fn new(opcode: Opcode) -> Instruction {
        Instruction { opcode }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::instruction::Opcode::IGL;

    #[test]
    fn test_create_hlt() {
        let opcode = Opcode::HLT;
        assert_eq!(opcode, Opcode::HLT);
    }

    #[test]
    fn test_create_instruction() {
        let instruction = Instruction::new(Opcode::HLT);
        assert_eq!(instruction.opcode, Opcode::HLT);
    }

    #[test]
    fn test_corresponding_enum_with_number() {
        let a: u8 = 200;
        let b = a.try_into().unwrap_or(Opcode::IGL);

        assert_eq!(b, IGL);
    }

    #[test]
    fn test_str_to_opcode() {
        let opcode = Opcode::from(CompleteStr("load"));
        assert_eq!(opcode, Opcode::LOAD);
        let opcode = Opcode::from(CompleteStr("illegal"));
        assert_eq!(opcode, Opcode::IGL);
    }
}
