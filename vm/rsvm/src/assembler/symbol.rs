#[derive(Debug, Clone)]
pub enum SymbolType {
    Label,
    Integer,
    IrString,
}

#[derive(Debug, Clone)]
pub struct Symbol {
    name: String,
    offset: Option<u32>,
    symbol_type: SymbolType,
}

#[derive(Debug)]
pub struct SymbolTable {
    pub symbols: Vec<Symbol>,
}

impl Symbol {
    pub fn new(name: String, symbol_type: SymbolType) -> Symbol {
        Symbol {
            name,
            symbol_type,
            offset: None,
        }
    }

    pub fn new_with_offset(name: String, symbol_type: SymbolType, offset: u32) -> Symbol {
        Symbol {
            name,
            symbol_type,
            offset: Some(offset),
        }
    }
}

impl SymbolTable {
    pub fn new() -> SymbolTable {
        SymbolTable { symbols: vec![] }
    }

    pub fn add_symbol(&mut self, s: Symbol) {
        self.symbols.push(s)
    }

    pub fn symbol_value(&self, s: &str) -> Option<u32> {
        for symbol in &self.symbols {
            if symbol.name == s {
                return symbol.offset;
            }
        }
        None
    }

    pub(super) fn has_symbol(&self, name: &str) -> bool {
        self.symbols.iter().find(|&sym| sym.name == name).is_some()
    }

    pub(super) fn set_symbol_offset(&mut self, name: &str, ro_offset: u32) -> bool {
        for symbol in &mut self.symbols {
            if symbol.name == name {
                symbol.offset = Some(ro_offset);
                return true;
            }
        }
        false
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_symbol_table() {
        let mut symbol_table = SymbolTable::new();
        let new_symbol = Symbol::new_with_offset("test".to_string(), SymbolType::Label, 12);
        symbol_table.add_symbol(new_symbol);
        assert_eq!(symbol_table.symbols.len(), 1);

        let v = symbol_table.symbol_value("test");
        assert!(v.is_some());
        assert_eq!(v.unwrap(), 12);

        let v = symbol_table.symbol_value("No-Exist");
        assert!(v.is_none());
    }
}
