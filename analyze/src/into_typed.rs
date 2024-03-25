use crate::{
    ast, infer,
    typed::{self, ReferenceType},
};
use kore::invariant;
use lang::{Node, NodeId, Range};
use std::{cell::OnceCell, ops::Deref};

pub trait IntoTyped: Sized {
    fn into_typed(self, strong: Visitor) -> typed::Program;
}

pub struct Visitor {
    next_node_id: usize,
    strong: infer::strong::Output,
}

impl Visitor {
    pub const fn new(strong: infer::strong::Output) -> Self {
        Self {
            next_node_id: 0,
            strong,
        }
    }

    fn next_node_id(&mut self) -> usize {
        let id = self.next_node_id;
        self.next_node_id += 1;
        id
    }

    fn next_type(&mut self) -> typed::ReferenceType {
        let id = NodeId(self.next_node_id());

        self.strong
            .types
            .get(&id)
            .and_then(OnceCell::get)
            .unwrap_or_else(|| invariant!("type not found"))
            .deref()
            .clone()
    }

    fn typed<T, R, F>(mut self, x: T, r: Range, f: F) -> (R, Self)
    where
        F: Fn(Node<T, ReferenceType>) -> R,
    {
        (f(Node(x, r, self.next_type())), self)
    }
}

impl ast::walk::Visit for Visitor {
    type Binding = typed::Binding;

    type Expression = typed::Expression;

    type Statement = typed::Statement;

    type Component = typed::Component;

    type TypeExpression = typed::TypeExpression;

    type Parameter = typed::Parameter;

    type Declaration = typed::Declaration;

    type Import = typed::Import;

    type Module = typed::Module;

    fn binding(self, x: ast::Binding, r: Range) -> (Self::Binding, Self) {
        (typed::Binding(Node::raw(x, r)), self)
    }

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        r: Range,
    ) -> (Self::Expression, Self) {
        self.typed(x, r, typed::Expression)
    }

    fn statement(self, x: ast::Statement<Self::Expression>, r: Range) -> (Self::Statement, Self) {
        self.typed(x, r, typed::Statement)
    }

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        r: Range,
    ) -> (Self::Component, Self) {
        self.typed(x, r, typed::Component)
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        r: Range,
    ) -> (Self::TypeExpression, Self) {
        self.typed(x, r, typed::TypeExpression)
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        r: Range,
    ) -> (Self::Parameter, Self) {
        self.typed(x, r, typed::Parameter)
    }

    fn declaration(
        self,
        x: ast::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        r: Range,
    ) -> (Self::Declaration, Self) {
        self.typed(x, r, typed::Declaration)
    }

    fn import(self, x: ast::Import, r: Range) -> (Self::Import, Self) {
        self.typed(x, r, typed::Import)
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        r: Range,
    ) -> (Self::Module, Self) {
        self.typed(x, r, typed::Module)
    }
}

#[cfg(test)]
mod tests {
    use super::IntoTyped;
    use crate::typed;
    use lang::{ast::walk::Walk, test::mock};

    impl IntoTyped for mock::Module {
        fn into_typed(self, strong: super::Visitor) -> typed::Program {
            typed::Program(self.walk(strong).0)
        }
    }
}
