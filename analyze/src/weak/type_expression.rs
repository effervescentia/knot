use crate::{
    ast::{self, walk},
    weak::{ToWeak, Weak, WeakRef},
};
use lang::types;

impl ToWeak for ast::TypeExpression<walk::NodeId> {
    fn to_weak(&self) -> WeakRef {
        (
            types::RefKind::Type,
            match self {
                Self::Primitive(x) => Weak::Type(match x {
                    ast::TypePrimitive::Nil => types::Type::Nil,
                    ast::TypePrimitive::Boolean => types::Type::Boolean,
                    ast::TypePrimitive::Integer => types::Type::Integer,
                    ast::TypePrimitive::Float => types::Type::Float,
                    ast::TypePrimitive::String => types::Type::String,
                    ast::TypePrimitive::Style => types::Type::Style,
                    ast::TypePrimitive::Element => types::Type::Element,
                }),

                Self::Identifier(..) => Weak::Infer,

                Self::Group(id) => Weak::Inherit(**id),

                Self::PropertyAccess(..) => Weak::Infer,

                Self::Function(params, x) => Weak::Type(types::Type::Function(params.clone(), **x)),
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{self, walk::NodeId},
        weak::{ToWeak, Weak},
    };
    use kore::str;
    use lang::types;

    #[test]
    fn primitive() {
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil).to_weak(),
            (types::RefKind::Type, Weak::Type(types::Type::Nil))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean).to_weak(),
            (types::RefKind::Type, Weak::Type(types::Type::Boolean))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer).to_weak(),
            (types::RefKind::Type, Weak::Type(types::Type::Integer))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Float).to_weak(),
            (types::RefKind::Type, Weak::Type(types::Type::Float))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::String).to_weak(),
            (types::RefKind::Type, Weak::Type(types::Type::String))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Style).to_weak(),
            (types::RefKind::Type, Weak::Type(types::Type::Style))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Element).to_weak(),
            (types::RefKind::Type, Weak::Type(types::Type::Element))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            ast::TypeExpression::Identifier(str!("foo")).to_weak(),
            (types::RefKind::Type, Weak::Infer)
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            ast::TypeExpression::Group(Box::new(NodeId(0))).to_weak(),
            (types::RefKind::Type, Weak::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            ast::TypeExpression::PropertyAccess(Box::new(NodeId(0)), str!("foo")).to_weak(),
            (types::RefKind::Type, Weak::Infer)
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            ast::TypeExpression::Function(vec![NodeId(0)], Box::new(NodeId(1))).to_weak(),
            (
                types::RefKind::Type,
                Weak::Type(types::Type::Function(vec![NodeId(0)], NodeId(1)))
            )
        );
    }
}
