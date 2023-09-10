use super::Type;
use crate::parser::{
    declaration::Declaration,
    expression::{ksx::KSX, Expression},
    module::Module,
    position::Decrement,
    range::Range,
    types::type_expression::TypeExpression,
};
use combine::Stream;
use std::{
    cell::RefCell,
    collections::{BTreeMap, HashMap},
    fmt::Debug,
};

#[derive(Debug, Clone, PartialEq)]
pub enum WeakType {
    Any,
    Number,
    Strong(Type<usize>),
    Reference(usize),
    NotFound(String),
}

#[derive(Debug, Clone, PartialEq)]
pub enum WeakRef {
    Type(WeakType),
    Value(WeakType),
}

pub struct WeakContext<'a> {
    types: &'a RefCell<BTreeMap<usize, WeakType>>,
    scope: HashMap<String, usize>,
}

impl<'a> WeakContext<'a> {
    fn new(types: &'a RefCell<BTreeMap<usize, WeakType>>) -> Self {
        Self {
            types,
            scope: HashMap::new(),
        }
    }

    fn child(&self) -> Self {
        Self {
            types: self.types,
            scope: self.scope.clone(),
        }
    }

    pub fn add_type(self, typ: WeakType) -> (usize, Self) {
        let id = self.types.borrow().len();
        self.types.borrow_mut().insert(id, typ);
        (id, self)
    }

    pub fn add_strong(self, typ: Type<usize>) -> (usize, Self) {
        self.add_type(WeakType::Strong(typ))
    }

    pub fn any(self) -> (usize, Self) {
        self.add_type(WeakType::Any)
    }

    pub fn get_id(self, name: &str) -> (Option<usize>, Self) {
        (self.scope.get(name).copied(), self)
    }

    pub fn resolve(self, name: &str) -> (usize, Self) {
        match self.get_id(&name) {
            (Some(id), ctx) => ctx.add_type(WeakType::Reference(id)),
            (None, ctx) => ctx.add_type(WeakType::NotFound(name.to_string())),
        }
    }

    pub fn bind(name: &str, (id, mut ctx): (usize, Self)) -> (usize, Self) {
        ctx.scope.insert(name.to_string(), id);
        (id, ctx)
    }

    pub fn refer((id, ctx): (usize, Self)) -> (usize, Self) {
        ctx.add_type(WeakType::Reference(id))
    }
}
