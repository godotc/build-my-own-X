use super::Token;
use nom::{digit, types::CompleteStr};

named!(pub register < CompleteStr, Token>,
    ws!(  // clear all white space
        do_parse!(
            tag!("$") >>
            reg_num: digit >>
            (
                Token::Register{
                    reg_num: reg_num.parse::<u8>().unwrap()
                }
            )
        )
    )
);

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_register() {
        let result = register(CompleteStr("$0"));
        assert_eq!(result.is_ok(), true);
        match result.unwrap().1 {
            Token::Register { reg_num } => assert_eq!(reg_num, 0.into()),
            _ => assert!(false),
        }

        let result = register(CompleteStr("0"));
        assert_eq!(result.is_ok(), false);
        let result = register(CompleteStr("$a"));
        assert_eq!(result.is_ok(), false);
    }
}
