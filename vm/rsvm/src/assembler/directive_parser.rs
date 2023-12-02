use super::{
    instruction_parser::{instruction_combined, AssemblerInstruction},
    label_parser::label_declaration,
};
use super::{operand_parsers::operand, Token};
use nom::{alpha1, types::CompleteStr};

named!(pub instruction<CompleteStr,AssemblerInstruction>,
    do_parse!(
        ins: alt!(
            instruction_combined|
            directive
        ) >>
        (
            ins
        )
    )
);

named!(pub directive_declaration<CompleteStr,Token>,
    do_parse!(
        tag!(".") >>
        name: alpha1 >>
        (
            Token::Directive { name: name.to_string() }
        )
    )
);

named!(pub directive<CompleteStr,AssemblerInstruction>,
    do_parse!(
        ins: alt!(
            directive_combined
        ) >>
        (
            ins
        )
    )
);

named!(directive_combined<CompleteStr, AssemblerInstruction>,
    ws!(
        do_parse!(
            l: opt!(label_declaration)>>
            name: directive_declaration >>
            o1: opt!(operand) >>
            o2: opt!(operand) >>
            o3: opt!(operand) >>
            (
                AssemblerInstruction{
                    opcode: None,
                    directive: Some(name),
                    label: l,
                    operand1: o1,
                    operand2: o2,
                    operand3: o3,
                }
            )
        )
    )
);

#[cfg(test)]
mod tests {

    use super::directive_combined;
    use crate::assembler::{instruction_parser::AssemblerInstruction, Token};
    use nom::types::CompleteStr;

    #[test]
    fn test_string_directive() {
        let ret = directive_combined(CompleteStr("test: .asciiz 'Hello'"));
        assert!(ret.is_ok());
        let (_, directive) = ret.unwrap();
        assert_eq!(
            directive,
            AssemblerInstruction {
                opcode: None,
                label: Some(Token::LabelDeclaration {
                    name: "test".to_string()
                }),
                directive: Some(Token::Directive {
                    name: "asciiz".to_string()
                }),
                operand1: Some(Token::IrString {
                    name: "Hello".to_string()
                }),
                operand2: None,
                operand3: None
            }
        )
    }
}
