use nom::types::CompleteStr;

use crate::token::Token;

named!(pub operator<CompleteStr, Token>,
    ws!(
        alt!(
            addition_operator|
            substraction_operator|
            multiplication_operator|
            division_operator
        )
    )
);

named!(pub addition_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("+") >>
            (
                Token::AdditionOperator
            )
        )
    )
);

named!(pub substraction_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("-") >>
            (
                Token::SubtractionOperator
            )
        )
    )
);

named!(pub multiplication_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("*") >>
            (
                Token::MultiplicationOperator
            )
        )
    )
);

named!(pub division_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("/") >>
            (
                Token::DivisionOperator
            )
        )
    )
);
