#[derive(Debug, PartialEq)]
pub enum Opcode {
    LOAD,

    ADD,
    SUB,
    MUL,
    DIV,

    JMP,

    HLT,
    IGL, // illegal
}

pub struct Instruction {
    opcode: Opcode,
}

impl From<u8> for Opcode {
    fn from(value: u8) -> Self {
        match value {
            0 => Self::HLT,
            1 => Self::LOAD,
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
}
