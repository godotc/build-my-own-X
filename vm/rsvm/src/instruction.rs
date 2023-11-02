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
    GTQ = 24,
    LTQ = 25,
    JEQ = 26,
    JNEQ = 27,

    JMPB = 79,
    JMP = 80,
    JMPF = 81,

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
            24 => Self::GTQ,
            25 => Self::LTQ,
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
}
