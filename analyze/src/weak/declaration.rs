use crate::{
    ast,
    weak::{ToWeak, Weak, WeakRef},
};
use lang::{types, NodeId};

impl ToWeak for ast::Declaration<String, NodeId, NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::TypeAlias { value, .. } => (types::RefKind::Type, Weak::Inherit(*value)),

            Self::Enumerated { variants, .. } => (
                types::RefKind::Mixed,
                Weak::Type(types::Type::Enumerated(types::Enumerated::Declaration(
                    variants.clone(),
                ))),
            ),

            Self::Constant {
                value_type, value, ..
            } => (
                types::RefKind::Value,
                Weak::Inherit(value_type.unwrap_or(*value)),
            ),

            Self::Function {
                parameters,
                body_type,
                body,
                ..
            } => (
                types::RefKind::Value,
                Weak::Type(types::Type::Function(
                    parameters.clone(),
                    body_type.unwrap_or(*body),
                )),
            ),

            Self::View { parameters, .. } => (
                types::RefKind::Value,
                Weak::Type(types::Type::View(parameters.clone())),
            ),

            Self::Module { value, .. } => (types::RefKind::Mixed, Weak::Inherit(*value)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        weak::{ToWeak, Weak},
    };
    use kore::str;
    use lang::{types, NodeId};

    #[test]
    fn type_alias() {
        assert_eq!(
            ast::Declaration::type_alias(ast::Storage::public(str!("Foo")), NodeId(0)).to_weak(),
            (types::RefKind::Type, Weak::Inherit(NodeId(0)))
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
                types::RefKind::Mixed,
                Weak::Type(types::Type::Enumerated(types::Enumerated::Declaration(
                    vec![(str!("Bar"), vec![NodeId(0), NodeId(1), NodeId(2)])]
                )))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            ast::Declaration::constant(ast::Storage::public(str!("FOO")), None, NodeId(0))
                .to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
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
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
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
                types::RefKind::Value,
                Weak::Type(types::Type::Function(vec![NodeId(0), NodeId(1)], NodeId(2)))
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
                types::RefKind::Value,
                Weak::Type(types::Type::Function(vec![NodeId(0), NodeId(1)], NodeId(2)))
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
                types::RefKind::Value,
                Weak::Type(types::Type::View(vec![NodeId(0), NodeId(1)]))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            ast::Declaration::module(ast::Storage::public(str!("foo")), NodeId(0)).to_weak(),
            (types::RefKind::Mixed, Weak::Inherit(NodeId(0)))
        );
    }
}
