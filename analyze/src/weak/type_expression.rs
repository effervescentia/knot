use crate::{
    ast,
    data::ScopedType,
    weak::{ToWeak, WeakRef},
};
use lang::{types, NodeId};

impl ToWeak for ast::TypeExpression<NodeId> {
    fn to_weak(&self) -> WeakRef {
        (
            types::RefKind::Type,
            match self {
                Self::Primitive(x) => Some(ScopedType::Type(match x {
                    ast::TypePrimitive::Nil => types::Type::Nil,
                    ast::TypePrimitive::Boolean => types::Type::Boolean,
                    ast::TypePrimitive::Integer => types::Type::Integer,
                    ast::TypePrimitive::Float => types::Type::Float,
                    ast::TypePrimitive::String => types::Type::String,
                    ast::TypePrimitive::Style => types::Type::Style,
                    ast::TypePrimitive::Element => types::Type::Element,
                })),

                Self::Identifier(..) => None,

                Self::Group(id) => Some(ScopedType::Inherit(**id)),

                Self::PropertyAccess(..) => None,

                Self::Function(params, x) => {
                    Some(ScopedType::Type(types::Type::Function(params.clone(), **x)))
                }
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{ast, data::ScopedType, weak::ToWeak};
    use kore::str;
    use lang::{types, NodeId};

    #[test]
    fn primitive() {
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::Nil))
            )
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::Boolean))
            )
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::Integer))
            )
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Float).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::Float))
            )
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::String).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::String))
            )
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Style).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::Style))
            )
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Element).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::Element))
            )
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            ast::TypeExpression::Identifier(str!("foo")).to_weak(),
            (types::RefKind::Type, None)
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            ast::TypeExpression::Group(Box::new(NodeId(0))).to_weak(),
            (types::RefKind::Type, Some(ScopedType::Inherit(NodeId(0))))
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            ast::TypeExpression::PropertyAccess(Box::new(NodeId(0)), str!("foo")).to_weak(),
            (types::RefKind::Type, None)
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            ast::TypeExpression::Function(vec![NodeId(0)], Box::new(NodeId(1))).to_weak(),
            (
                types::RefKind::Type,
                Some(ScopedType::Type(types::Type::Function(
                    vec![NodeId(0)],
                    NodeId(1)
                )))
            )
        );
    }
}
