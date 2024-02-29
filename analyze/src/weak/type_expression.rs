use crate::ast;
use lang::{
    types::{RefKind, Type},
    NodeId,
};

impl super::ToWeak for ast::TypeExpression<NodeId> {
    fn to_weak(&self) -> super::Ref {
        (
            RefKind::Type,
            match self {
                Self::Primitive(x) => super::Type::Local(match x {
                    ast::TypePrimitive::Nil => Type::Nil,
                    ast::TypePrimitive::Boolean => Type::Boolean,
                    ast::TypePrimitive::Integer => Type::Integer,
                    ast::TypePrimitive::Float => Type::Float,
                    ast::TypePrimitive::String => Type::String,
                    ast::TypePrimitive::Style => Type::Style,
                    ast::TypePrimitive::Element => Type::Element,
                }),

                Self::Group(id) => super::Type::Inherit(**id),

                Self::Identifier(..) | Self::PropertyAccess(..) => super::Type::Infer,

                Self::Function(params, x) => {
                    super::Type::Local(Type::Function(params.clone(), **x))
                }
            },
        )
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
        types::{RefKind, Type},
        NodeId,
    };

    #[test]
    fn primitive() {
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil).to_weak(),
            (RefKind::Type, weak::Type::Local(Type::Nil))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean).to_weak(),
            (RefKind::Type, weak::Type::Local(Type::Boolean))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer).to_weak(),
            (RefKind::Type, weak::Type::Local(Type::Integer))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Float).to_weak(),
            (RefKind::Type, weak::Type::Local(Type::Float))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::String).to_weak(),
            (RefKind::Type, weak::Type::Local(Type::String))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Style).to_weak(),
            (RefKind::Type, weak::Type::Local(Type::Style))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Element).to_weak(),
            (RefKind::Type, weak::Type::Local(Type::Element))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            ast::TypeExpression::Identifier(str!("foo")).to_weak(),
            (RefKind::Type, weak::Type::Infer)
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            ast::TypeExpression::Group(Box::new(NodeId(0))).to_weak(),
            (RefKind::Type, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            ast::TypeExpression::PropertyAccess(Box::new(NodeId(0)), str!("foo")).to_weak(),
            (RefKind::Type, weak::Type::Infer)
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            ast::TypeExpression::Function(vec![NodeId(0)], Box::new(NodeId(1))).to_weak(),
            (
                RefKind::Type,
                weak::Type::Local(Type::Function(vec![NodeId(0)], NodeId(1)))
            )
        );
    }
}
