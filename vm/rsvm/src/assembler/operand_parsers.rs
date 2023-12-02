use nom::{digit, types::CompleteStr};

use super::{label_parser::label_usage, register_parsers::register, Token};

named!(pub operand<CompleteStr, Token>,
    alt!(
        integer_operand|
        label_usage|
        register|
        irstring
    )
);

named!(integer_operand < CompleteStr, Token>,
    ws!(  // clear all white space
        do_parse!(
            tag!("#") >>
            value: digit >>
            (
                Token::IntegerOperand {
                    value: value.parse::<i32>().unwrap()
                }
            )
        )
    )
);

named!(irstring<CompleteStr,Token>,
do_parse!(
    tag!("'")>>
    content: take_until!("'")>>
    tag!("'")>>
    (
        Token::IrString{name:content.to_string()}
    )

));

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_parse_integer_operand() {
        let result = integer_operand(CompleteStr("#10"));
        assert!(result.is_ok());

        let (rest, value) = result.unwrap();
        assert_eq!(rest, CompleteStr(""));
        assert_eq!(value, Token::IntegerOperand { value: 10 });

        let result = integer_operand(CompleteStr("10"));
        assert!(result.is_err());
    }

    #[test]
    fn test_parse_string_operand() {
        let result = irstring(CompleteStr("'The Content'"));
        assert!(result.is_ok());
        assert_eq!(
            result.unwrap().1,
            Token::IrString {
                name: "The Content".to_string()
            }
        )
    }
}
