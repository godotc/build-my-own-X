use std::{collections::HashMap, iter::Map, vec};

pub type AttrMap = HashMap<String, String>;

#[derive(Debug)]
pub struct ElementData {
    pub tag_name: String,
    pub attributes: AttrMap,
}

#[derive(Debug)]
pub enum NodeType {
    Text(String),
    Element(ElementData),
}

#[derive(Debug)]
pub struct Node {
    pub children: Vec<Node>,
    pub node_type: NodeType,
}

pub fn text(data: String) -> Node {
    Node {
        children: vec![],
        node_type: NodeType::Text(data),
    }
}

pub fn elem(name: String, attrs: AttrMap, children: Vec<Node>) -> Node {
    Node {
        children,
        node_type: NodeType::Element(ElementData {
            tag_name: name,
            attributes: attrs,
        }),
    }
}
