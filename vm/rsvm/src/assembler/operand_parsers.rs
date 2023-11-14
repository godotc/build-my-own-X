use nom::{digit, types::CompleteStr};

use super::Token;

named!(pub integer_operand < CompleteStr, Token>,
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
}
