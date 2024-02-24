pub struct Parser {
    pos: usize,
    input: String,
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
}
