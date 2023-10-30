#![allow(dead_code)]

use crate::ast::{
    Declaration, DeclarationNode, ExpressionNode, KSXNode, ModuleNode, StatementNode,
    TypeExpressionNode,
};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct MockRange;

/// node factories
pub mod n {
    use super::*;
    use crate::ast::{
        DeclarationNodeValue, ExpressionNodeValue, ImportNode, ImportNodeValue, KSXNodeValue,
        ModuleNodeValue, ParameterNode, ParameterNodeValue, StatementNodeValue,
        TypeExpressionNodeValue,
    };

    pub const fn x(x: ExpressionNodeValue<MockRange, ()>) -> ExpressionNode<MockRange, ()> {
        ExpressionNode::raw(x, MockRange)
    }

    pub const fn s(x: StatementNodeValue<MockRange, ()>) -> StatementNode<MockRange, ()> {
        StatementNode::raw(x, MockRange)
    }

    pub const fn kx(x: KSXNodeValue<MockRange, ()>) -> KSXNode<MockRange, ()> {
        KSXNode::raw(x, MockRange)
    }

    pub const fn p(x: ParameterNodeValue<MockRange, ()>) -> ParameterNode<MockRange, ()> {
        ParameterNode::raw(x, MockRange)
    }

    pub const fn tx(
        x: TypeExpressionNodeValue<MockRange, ()>,
    ) -> TypeExpressionNode<MockRange, ()> {
        TypeExpressionNode::raw(x, MockRange)
    }

    pub const fn d(x: DeclarationNodeValue<MockRange, ()>) -> DeclarationNode<MockRange, ()> {
        DeclarationNode::raw(x, MockRange)
    }

    pub const fn i(x: ImportNodeValue) -> ImportNode<MockRange, ()> {
        ImportNode::raw(x, MockRange)
    }

    pub const fn m(x: ModuleNodeValue<MockRange, ()>) -> ModuleNode<MockRange, ()> {
        ModuleNode::raw(x)
    }
}

/// ast factories
pub mod a {
    use super::*;
    use crate::ast::storage::{Storage, Visibility};

    pub fn type_<E, P, M, T>(name: &str, value: T) -> Declaration<E, P, M, T> {
        Declaration::TypeAlias {
            name: Storage(Visibility::Public, name.to_owned()),
            value,
        }
    }

    pub fn enum_<E, P, M, T>(
        name: &str,
        variants: Vec<(String, Vec<T>)>,
    ) -> Declaration<E, P, M, T> {
        Declaration::Enumerated {
            name: Storage(Visibility::Public, name.to_owned()),
            variants,
        }
    }

    pub fn const_<E, P, M, T>(
        name: &str,
        value_type: Option<T>,
        value: E,
    ) -> Declaration<E, P, M, T> {
        Declaration::Constant {
            name: Storage(Visibility::Public, name.to_owned()),
            value_type,
            value,
        }
    }

    pub fn func_<E, P, M, T>(
        name: &str,
        parameters: Vec<P>,
        body_type: Option<T>,
        body: E,
    ) -> Declaration<E, P, M, T> {
        Declaration::Function {
            name: Storage(Visibility::Public, name.to_owned()),
            parameters,
            body_type,
            body,
        }
    }

    pub fn view<E, P, M, T>(name: &str, parameters: Vec<P>, body: E) -> Declaration<E, P, M, T> {
        Declaration::View {
            name: Storage(Visibility::Public, name.to_owned()),
            parameters,
            body,
        }
    }

    pub fn module<E, P, M, T>(name: &str, value: M) -> Declaration<E, P, M, T> {
        Declaration::Module {
            name: Storage(Visibility::Public, name.to_owned()),
            value,
        }
    }
}
