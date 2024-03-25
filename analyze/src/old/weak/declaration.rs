use crate::ast;
use lang::{
    types::{Enumerated, RefKind, Type},
    NodeId,
};
use std::ops::Deref;

impl super::ToWeak for ast::Declaration<String, NodeId, NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> super::Ref {
        match self {
            Self::TypeAlias { value, .. } => (RefKind::Type, super::Type::Inherit(*value)),

            Self::Enumerated { variants, .. } => (
                RefKind::Mixed,
                super::Type::Local(Type::Enumerated(Enumerated::Declaration(variants.clone()))),
            ),

            Self::Constant {
                value_type, value, ..
            } => (
                RefKind::Value,
                value_type
                    .deref()
                    .map(super::Type::inherit_from_type)
                    .unwrap_or(super::Type::Inherit(*value)),
            ),

            Self::Function {
                parameters,
                body_type,
                body,
                ..
            } => (
                RefKind::Value,
                super::Type::Local(Type::Function(
                    parameters.clone(),
                    body_type.unwrap_or(*body),
                )),
            ),

            Self::View { parameters, .. } => (
                RefKind::Value,
                super::Type::Local(Type::View(parameters.clone())),
            ),

            Self::Module { value, .. } => (RefKind::Mixed, super::Type::Inherit(*value)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        weak::{self, ToWeak},
    };
    use kore::str;
    use lang::{
        types::{Enumerated, RefKind, Type},
        NodeId,
    };

    #[test]
    fn type_alias() {
        assert_eq!(
            ast::Declaration::type_alias(ast::Storage::public(str!("Foo")), NodeId(0)).to_weak(),
            (RefKind::Type, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            ast::Declaration::enumerated(
                ast::Storage::public(str!("Foo")),
                vec![(str!("Bar"), vec![NodeId(0), NodeId(1), NodeId(2)])]
            )
            .to_weak(),
            (
                RefKind::Mixed,
                weak::Type::Local(Type::Enumerated(Enumerated::Declaration(vec![(
                    str!("Bar"),
                    vec![NodeId(0), NodeId(1), NodeId(2)]
                )])))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            ast::Declaration::constant(ast::Storage::public(str!("FOO")), None, NodeId(0))
                .to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn constant_with_type() {
        assert_eq!(
            ast::Declaration::constant(
                ast::Storage::public(str!("FOO")),
                Some(NodeId(0)),
                NodeId(1)
            )
            .to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            ast::Declaration::function(
                ast::Storage::public(str!("foo")),
                vec![NodeId(0), NodeId(1)],
                None,
                NodeId(2)
            )
            .to_weak(),
            (
                RefKind::Value,
                weak::Type::Local(Type::Function(vec![NodeId(0), NodeId(1)], NodeId(2)))
            )
        );
    }

    #[test]
    fn function_with_typedef() {
        assert_eq!(
            ast::Declaration::function(
                ast::Storage::public(str!("foo")),
                vec![NodeId(0), NodeId(1)],
                Some(NodeId(2)),
                NodeId(3)
            )
            .to_weak(),
            (
                RefKind::Value,
                weak::Type::Local(Type::Function(vec![NodeId(0), NodeId(1)], NodeId(2)))
            )
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            ast::Declaration::view(
                ast::Storage::public(str!("Foo")),
                vec![NodeId(0), NodeId(1)],
                NodeId(2)
            )
            .to_weak(),
            (
                RefKind::Value,
                weak::Type::Local(Type::View(vec![NodeId(0), NodeId(1)]))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            ast::Declaration::module(ast::Storage::public(str!("foo")), NodeId(0)).to_weak(),
            (RefKind::Mixed, weak::Type::Inherit(NodeId(0)))
        );
    }
}
