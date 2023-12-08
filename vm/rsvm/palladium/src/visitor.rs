use super::token::Token;

pub trait Visitor {
    fn visit_token(&mut self, node: &Token);
}
