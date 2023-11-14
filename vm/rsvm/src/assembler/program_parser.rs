use nom::types::CompleteStr;

use super::instruction_parser::{instruction_one, AssemblerInstruction};

pub struct Program {
    instructions: Vec<AssemblerInstruction>,
}

named!(pub program<CompleteStr, Program>,
    do_parse!(
        instructions: many1!(instruction_one) >>
        (
            Program{
                instructions,
            }
        )
    )
);

impl Program {
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut program = vec![];
        for inst in &self.instructions {
            program.append(&mut inst.to_bytes())
        }
        program
    }
}

#[cfg(test)]
mod tests {
    use super::program;

    #[test]
    fn test_parse_program() {
        let result = program("load $0 #100\n".into());
        assert!(result.is_ok());
        let (lefts, p) = result.unwrap();
        assert!(lefts == "".into());
        assert!(1 == p.instructions.len());

        // TODO: Figure out an ergonomic way to test the AssemblerInstruction returned (by blog)
    }

    #[test]
    fn test_program_to_bytes() {
        let result = program("load $0 #100\n".into());
        assert!(result.is_ok());
        let (_, p) = result.unwrap();
        let bytecodes = p.to_bytes();
        assert_eq!(bytecodes.len(), 4);
        println!("{:?}", bytecodes);
    }
}
