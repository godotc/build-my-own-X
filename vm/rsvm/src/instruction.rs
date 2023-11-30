use nom::types::CompleteStr;

macro_rules! declare_opcodes {
    ($
        (
            ($instruction:ident, $binary:tt)
        ), +
    ) => {
        #[derive(Debug, PartialEq, Clone, Copy)]
        pub enum Opcode{
            $($instruction,)+
            IGL,
        }

        impl Into<u8> for Opcode {
            fn into(self) -> u8 {
                match self{
                    $(Self::$instruction => $binary,)+
                    _ => {
                        // println!("Non-opcode found in opcode field! ");
                        // std::process::exit(1);
                        255
                    }
                }
            }
        }

        impl From<u8> for Opcode {
            fn from(value: u8) -> Self {
                match value {
                    $($binary => Self::$instruction,)+
                    _ => Self::IGL,
                }
            }
        }

        impl<'a> From<CompleteStr<'a>> for Opcode {
            fn from(value: CompleteStr<'a>) -> Self {
                match value.to_uppercase().as_str() {
                    $(
                        stringify!($instruction) => Opcode::$instruction,
                    )+
                    _ => Opcode::IGL,
                }
            }
        }



    };
}
// declare_opcodes!( (LOAD, 0))
// #[derive(Debug, PartialEq, Clone, Copy)]
// pub enum Opcode {
//     LOAD,
//     IGL,
// }
// impl Into<u8> for Opcode {
//     fn into(self) -> u8 {
//         match self {
//             Self::LOAD => 0,
//             _ => 255,
//         }
//     }
// }
// impl From<u8> for Opcode {
//     fn from(value: u8) -> Self {
//         match value {
//             0 => Self::LOAD,
//             _ => Self::IGL,
//         }
//     }
// }
// impl<'a> From<CompleteStr<'a>> for Opcode {
//     fn from(value: CompleteStr<'a>) -> Self {
//         match value.to_uppercase().as_str() {
//             stringify!(LOAD) => Opcode::LOAD,
//             _ => Opcode::IGL,
//         }
//     }
// }

declare_opcodes!(
    (LOAD, 0),
    //
    (ADD, 10),
    (SUB, 11),
    (MUL, 12),
    (DIV, 13),
    (INC, 14),
    (DEC, 15),
    //
    (EQ, 20),
    (NEQ, 21),
    (GT, 22),
    (LT, 23),
    (GTE, 24),
    (LTE, 25),
    (JEQ, 26),
    (JNEQ, 27),
    //
    (JMPB, 79),
    (JMP, 80),
    (JMPF, 81),
    //
    (NOP, 98),
    (ALOC, 100),
    //
    (HLT, 254) // IGL -> 255
);

pub struct Instruction {
    opcode: Opcode,
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
