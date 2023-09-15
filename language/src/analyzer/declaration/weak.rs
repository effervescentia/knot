use crate::{
    analyzer::{infer::weak::ToWeak, RefKind, Type, WeakRef, WeakType},
    parser::declaration::Declaration,
};

impl ToWeak for Declaration<usize, usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Declaration::TypeAlias { value, .. } => (RefKind::Type, WeakType::Reference(*value)),

            Declaration::Enumerated { variants, .. } => (
                RefKind::Value,
                WeakType::Strong(Type::Enumerated(
                    variants
                        .into_iter()
                        .map(|(name, parameters)| {
                            (
                                name.clone(),
                                parameters.into_iter().map(|x| *x).collect::<Vec<_>>(),
                            )
                        })
                        .collect::<Vec<_>>(),
                )),
            ),

            Declaration::Constant {
                value_type, value, ..
            } => (
                RefKind::Value,
                WeakType::Reference(value_type.unwrap_or(*value)),
            ),

            Declaration::Function { .. } => (RefKind::Value, WeakType::Any),

            Declaration::View { .. } => (RefKind::Value, WeakType::Any),

            Declaration::Module { .. } => (RefKind::Value, WeakType::Any),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
        parser::{
            declaration::parameter::Parameter,
            expression::{primitive::Primitive, Expression},
            module::{
                import::{Import, Source, Target},
                Module, ModuleNode,
            },
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::BTreeMap;
}
