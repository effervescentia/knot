use crate::{
    analyzer::{infer::weak::ToWeak, RefKind, Type, WeakRef, WeakType},
    parser::types::type_expression::TypeExpression,
};

impl ToWeak for TypeExpression<usize> {
    fn to_weak(&self) -> WeakRef {
        (
            RefKind::Type,
            match self {
                TypeExpression::Nil => WeakType::Strong(Type::Nil),
                TypeExpression::Boolean => WeakType::Strong(Type::Boolean),
                TypeExpression::Integer => WeakType::Strong(Type::Integer),
                TypeExpression::Float => WeakType::Strong(Type::Float),
                TypeExpression::String => WeakType::Strong(Type::String),
                TypeExpression::Style => WeakType::Strong(Type::Style),
                TypeExpression::Element => WeakType::Strong(Type::Element),

                TypeExpression::Identifier(..) => WeakType::Any,

                TypeExpression::Group(id) => WeakType::Reference(**id),

                TypeExpression::DotAccess(..) => WeakType::Any,

                TypeExpression::Function(..) => WeakType::Any,
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{infer::weak::ToWeak, RefKind, Type, WeakType},
        parser::types::type_expression::TypeExpression,
    };

    #[test]
    fn primitive() {
        assert_eq!(
            TypeExpression::Nil.to_weak(),
            (RefKind::Type, WeakType::Strong(Type::Nil))
        );
        assert_eq!(
            TypeExpression::Boolean.to_weak(),
            (RefKind::Type, WeakType::Strong(Type::Boolean))
        );
        assert_eq!(
            TypeExpression::Integer.to_weak(),
            (RefKind::Type, WeakType::Strong(Type::Integer))
        );
        assert_eq!(
            TypeExpression::Float.to_weak(),
            (RefKind::Type, WeakType::Strong(Type::Float))
        );
        assert_eq!(
            TypeExpression::String.to_weak(),
            (RefKind::Type, WeakType::Strong(Type::String))
        );
        assert_eq!(
            TypeExpression::Style.to_weak(),
            (RefKind::Type, WeakType::Strong(Type::Style))
        );
        assert_eq!(
            TypeExpression::Element.to_weak(),
            (RefKind::Type, WeakType::Strong(Type::Element))
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            TypeExpression::Identifier(String::from("foo")).to_weak(),
            (RefKind::Type, WeakType::Any)
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            TypeExpression::Group(Box::new(0)).to_weak(),
            (RefKind::Type, WeakType::Reference(0))
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            TypeExpression::DotAccess(Box::new(0), String::from("foo")).to_weak(),
            (RefKind::Type, WeakType::Any)
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            TypeExpression::Function(vec![0], Box::new(1)).to_weak(),
            (RefKind::Type, WeakType::Any)
        );
    }
}
