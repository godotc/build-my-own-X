use super::{expression_parser::expression, token::Token};
use nom::{named, types::CompleteStr, ws};

named!(pub program<CompleteStr, Token>,
    ws!(
        do_parse!(
            expressions: many1!(expression) >>
            (
                Token::Program { expressions }
            )
        )
    )
);

mod tests {
    use super::*;
    #[test]
    fn test_parse_program() {
        let test_program = CompleteStr("1+2");
        let result = program(test_program);
        assert_eq!(result.is_ok(), true);
    }
}
