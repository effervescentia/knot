use crate::ast2::{
    self,
    typed::{self, TypeRef},
    walk::{NodeId, Visit, Walk},
    Range,
};
use kore::invariant;
use std::collections::HashMap;

struct Visitor<'a> {
    next_id: usize,
    types: HashMap<NodeId, typed::TypeRef<'a>>,
}

impl<'a> Visitor<'a> {
    const fn new(types: HashMap<NodeId, typed::TypeRef<'a>>) -> Self {
        Self { next_id: 0, types }
    }

    fn next(&mut self) -> typed::TypeRef<'a> {
        let id = NodeId(self.next_id);
        self.next_id += 1;

        self.types
            .remove(&id)
            .unwrap_or_else(|| invariant!("type does not exist by id {id:?}"))
    }

    fn wrap<Value, Result, F>(mut self, value: Value, range: Range, f: F) -> (Result, Self)
    where
        F: Fn(typed::Node<Value, typed::TypeRef<'a>>) -> Result,
    {
        (f(typed::Node::new(value, self.next(), range)), self)
    }
}

impl<'a> Visit for Visitor<'a> {
    type Expression = typed::Expression<TypeRef<'a>>;
    type Statement = typed::Statement<TypeRef<'a>>;
    type Component = typed::Component<TypeRef<'a>>;
    type TypeExpression = typed::TypeExpression<TypeRef<'a>>;
    type Parameter = typed::Parameter<TypeRef<'a>>;
    type Declaration = typed::Declaration<TypeRef<'a>>;
    type Import = typed::Import<TypeRef<'a>>;
    type Module = typed::Module<TypeRef<'a>>;

    fn expression(
        self,
        x: ast2::Expression<Self::Expression, Self::Statement, Self::Component>,
        r: Range,
    ) -> (Self::Expression, Self) {
        self.wrap(x, r, typed::Expression)
    }

    fn statement(self, x: ast2::Statement<Self::Expression>, r: Range) -> (Self::Statement, Self) {
        self.wrap(x, r, typed::Statement)
    }

    fn component(
        self,
        x: ast2::Component<Self::Expression, Self::Component>,
        r: Range,
    ) -> (Self::Component, Self) {
        self.wrap(x, r, typed::Component)
    }

    fn type_expression(
        self,
        x: ast2::TypeExpression<Self::TypeExpression>,
        r: Range,
    ) -> (Self::TypeExpression, Self) {
        self.wrap(x, r, typed::TypeExpression)
    }

    fn parameter(
        self,
        x: ast2::Parameter<Self::Expression, Self::TypeExpression>,
        r: Range,
    ) -> (Self::Parameter, Self) {
        self.wrap(x, r, typed::Parameter)
    }

    fn declaration(
        self,
        x: ast2::Declaration<Self::Expression, Self::Parameter, Self::Module, Self::TypeExpression>,
        r: Range,
    ) -> (Self::Declaration, Self) {
        self.wrap(x, r, typed::Declaration)
    }

    fn import(self, x: ast2::Import, r: Range) -> (Self::Import, Self) {
        self.wrap(x, r, typed::Import)
    }

    fn module(
        self,
        x: ast2::Module<Self::Import, Self::Declaration>,
        r: Range,
    ) -> (Self::Module, Self) {
        self.wrap(x, r, typed::Module)
    }
}

#[allow(clippy::multiple_inherent_impl)]
impl super::Program {
    pub fn augment(self, types: HashMap<NodeId, typed::TypeRef>) -> typed::Program<typed::TypeRef> {
        let (result, _) = self.0.walk(Visitor::new(types));

        typed::Program(result)
    }
}
