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
