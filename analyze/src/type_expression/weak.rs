use crate::{
    infer::weak::{ToWeak, Weak, WeakRef},
    RefKind, Type,
};
use lang::ast::TypeExpression;

impl ToWeak for TypeExpression<usize> {
    fn to_weak(&self) -> WeakRef {
        (
            RefKind::Type,
            match self {
                Self::Nil => Weak::Type(Type::Nil),
                Self::Boolean => Weak::Type(Type::Boolean),
                Self::Integer => Weak::Type(Type::Integer),
                Self::Float => Weak::Type(Type::Float),
                Self::String => Weak::Type(Type::String),
                Self::Style => Weak::Type(Type::Style),
                Self::Element => Weak::Type(Type::Element),

                Self::Identifier(..) => Weak::Infer,

                Self::Group(id) => Weak::Inherit(**id),

                Self::DotAccess(..) => Weak::Infer,

                Self::Function(params, x) => Weak::Type(Type::Function(params.clone(), **x)),
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        infer::weak::{ToWeak, Weak},
        RefKind, Type,
    };
    use lang::ast::TypeExpression;

    #[test]
    fn primitive() {
        assert_eq!(
            TypeExpression::Nil.to_weak(),
            (RefKind::Type, Weak::Type(Type::Nil))
        );
        assert_eq!(
            TypeExpression::Boolean.to_weak(),
            (RefKind::Type, Weak::Type(Type::Boolean))
        );
        assert_eq!(
            TypeExpression::Integer.to_weak(),
            (RefKind::Type, Weak::Type(Type::Integer))
        );
        assert_eq!(
            TypeExpression::Float.to_weak(),
            (RefKind::Type, Weak::Type(Type::Float))
        );
        assert_eq!(
            TypeExpression::String.to_weak(),
            (RefKind::Type, Weak::Type(Type::String))
        );
        assert_eq!(
            TypeExpression::Style.to_weak(),
            (RefKind::Type, Weak::Type(Type::Style))
        );
        assert_eq!(
            TypeExpression::Element.to_weak(),
            (RefKind::Type, Weak::Type(Type::Element))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            TypeExpression::Identifier(String::from("foo")).to_weak(),
            (RefKind::Type, Weak::Infer)
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            TypeExpression::Group(Box::new(0)).to_weak(),
            (RefKind::Type, Weak::Inherit(0))
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            TypeExpression::DotAccess(Box::new(0), String::from("foo")).to_weak(),
            (RefKind::Type, Weak::Infer)
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            TypeExpression::Function(vec![0], Box::new(1)).to_weak(),
            (RefKind::Type, Weak::Type(Type::Function(vec![0], 1)))
        );
    }
}
