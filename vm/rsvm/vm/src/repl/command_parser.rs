pub struct CommandParser {}

impl CommandParser {
    pub fn tokenize(input: &str) -> Vec<&str> {
        input.split_whitespace().collect()
    }
}
