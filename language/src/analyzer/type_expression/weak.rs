use crate::{
    analyzer::{infer::weak::ToWeak, RefKind, Type, Weak, WeakRef},
    parser::types::type_expression::TypeExpression,
};

impl ToWeak for TypeExpression<usize> {
    fn to_weak(&self) -> WeakRef {
        (
            RefKind::Type,
            match self {
                TypeExpression::Nil => Weak::Type(Type::Nil),
                TypeExpression::Boolean => Weak::Type(Type::Boolean),
                TypeExpression::Integer => Weak::Type(Type::Integer),
                TypeExpression::Float => Weak::Type(Type::Float),
                TypeExpression::String => Weak::Type(Type::String),
                TypeExpression::Style => Weak::Type(Type::Style),
                TypeExpression::Element => Weak::Type(Type::Element),

                TypeExpression::Identifier(..) => Weak::Unknown,

                TypeExpression::Group(id) => Weak::Inherit(**id),

                TypeExpression::DotAccess(..) => Weak::Unknown,

                TypeExpression::Function(params, x) => {
                    Weak::Type(Type::Function(params.clone(), **x))
                }
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{infer::weak::ToWeak, RefKind, Type, Weak},
        parser::types::type_expression::TypeExpression,
    };

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
            (RefKind::Type, Weak::Unknown)
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
            (RefKind::Type, Weak::Unknown)
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
