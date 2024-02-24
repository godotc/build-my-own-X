use crate::parser::Parser;

struct SimpleSelector {
    tag_name: Option<String>,
    id: Option<String>,
    class: Vec<String>,
}

enum Selector {
    Simple(SimpleSelector),
}

enum Unit {
    Px,
}

// struct Color(u8, u8, u8, u8);
struct Color {
    r: u8,
    g: u8,
    b: u8,
    a: u8,
}

enum Value {
    Keyword(String),
    Length(f32, Unit),
    ColorValue(Color),
}

struct Declaration {
    name: String,
    value: Value,
}

struct Rule {
    selectors: Vec<Selector>,
    declaratiojns: Vec<Declaration>,
}

struct StyleSheet {
    rules: Vec<Rule>,
}

struct CssParser {
    parent: Parser,
}

impl Parser {
    fn parse_simple_selector(&mut self) -> SimpleSelector {
        let mut selector = SimpleSelector {
            tag_name: None,
            id: None,
            class: vec![],
        };
    }
}
