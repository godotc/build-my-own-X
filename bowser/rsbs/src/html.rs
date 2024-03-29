use std::collections::HashMap;

use crate::dom;

struct Parser {
    pos: usize,
    input: String,
}

pub fn parse(source: String) -> dom::Node {
    let mut nodes = Parser {
        pos: 0,
        input: source,
    }
    .parse_nodes();

    // if has root node, return it, or creat a wraper root node
    if nodes.len() == 1 {
        return nodes.swap_remove(0);
    } else {
        return dom::elem("html".to_string(), HashMap::new(), nodes);
    }
}

impl Parser {
    fn next_char(&self) -> char {
        self.input[self.pos..].chars().next().unwrap()
    }

    fn start_with(&self, s: &str) -> bool {
        self.input[self.pos..].starts_with(s)
    }

    fn eof(&self) -> bool {
        self.pos >= self.input.len()
    }

    fn consue_char(&mut self) -> char {
        let mut iter = self.input[self.pos..].char_indices();
        let (_, cur_char) = iter.next().unwrap();
        let (next_pos, _) = iter.next().unwrap_or((1, ' '));
        self.pos += next_pos;
        cur_char
    }

    fn consume_while<F>(&mut self, test: F) -> String
    where
        F: Fn(char) -> bool,
    {
        let mut result = String::new();
        while !self.eof() && test(self.next_char()) {
            result.push(self.consue_char());
        }
        return result;
    }

    fn consume_whitespace(&mut self) {
        self.consume_while(|c| c == ' ');
    }

    fn parse_tag_name(&mut self) -> String {
        self.consume_while(|c| match c {
            'a'..='z' | 'A'..='Z' | '0'..='9' => true,
            _ => false,
        })
    }

    fn consume_comment(&mut self) {
        loop {
            if self.start_with("-->") {
                self.consume_while(|c| c == '>');
                break;
            }
            self.consue_char();
        }
    }

    fn parse_nodes(&mut self) -> Vec<dom::Node> {
        let mut nodes = vec![];
        loop {
            self.consume_whitespace();
            if self.eof() || self.start_with("</") {
                break;
            }
            let node = self.parse_node();
            nodes.push(node);
        }
        nodes
    }

    // a single node
    fn parse_node(&mut self) -> dom::Node {
        match self.next_char() {
            '<' => self.parse_element(),
            _ => self.parse_text(),
        }
    }

    fn parse_element(&mut self) -> dom::Node {
        if (self.start_with("<!--")) {
            self.consume_comment();
            self.consume_whitespace();
        }

        assert!(self.consue_char() == '<');
        let tag_name = self.parse_tag_name();
        let attrs = self.parse_attributes();
        assert!(self.consue_char() == '>');

        // content
        let children = self.parse_nodes();

        assert!(self.consue_char() == '<');
        assert!(self.consue_char() == '/');
        assert!(self.parse_tag_name() == tag_name);
        assert!(self.consue_char() == '>');

        dom::elem(tag_name, attrs, children)
    }

    fn parse_text(&mut self) -> dom::Node {
        dom::text(self.consume_while(|c| c != '<'))
    }

    fn parse_attributes(&mut self) -> dom::AttrMap {
        let mut attributes = HashMap::new();
        loop {
            self.consume_whitespace();
            if self.next_char() == '>' {
                break;
            }
            let (name, val) = self.parse_attr();
            attributes.insert(name, val);
        }
        attributes
    }

    fn parse_attr(&mut self) -> (String, String) {
        let name = self.parse_tag_name();
        assert!(self.consue_char() == '=');
        let val = self.parse_attr_value();
        (name, val)
    }

    fn parse_attr_value(&mut self) -> String {
        let open_quote = self.consue_char();
        assert!(open_quote == '"' || open_quote == '\'');
        let val = self.consume_while(|c| c != open_quote);
        assert!(self.consue_char() == open_quote);
        val
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        dom::{Node, NodeType},
        html::parse,
    };

    use super::Parser;

    #[test]
    fn test_parse_html() {
        let source = "<html> <body> hello world! </body> </html>";
        let root_node: Node = parse(source.to_string());
        println!("{:#?}", root_node);
        match root_node.node_type {
            NodeType::Element(elem_data) => {
                assert!(elem_data.tag_name == "html".to_string());
            }
            _ => {}
        };
    }
}
