use crate::{
    infer::weak::{ToWeak, Weak, WeakRef},
    RefKind, Type,
};
use lang::ast::Declaration;

impl ToWeak for Declaration<usize, usize, usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Declaration::TypeAlias { value, .. } => (RefKind::Type, Weak::Inherit(*value)),

            Declaration::Enumerated { variants, .. } => (
                RefKind::Mixed,
                Weak::Type(Type::Enumerated(variants.clone())),
            ),

            Declaration::Constant {
                value_type, value, ..
            } => (RefKind::Value, Weak::Inherit(value_type.unwrap_or(*value))),

            Declaration::Function {
                parameters,
                body_type,
                body,
                ..
            } => (
                RefKind::Value,
                Weak::Type(Type::Function(
                    parameters.clone(),
                    body_type.unwrap_or(*body),
                )),
            ),

            Declaration::View { parameters, .. } => {
                (RefKind::Value, Weak::Type(Type::View(parameters.clone())))
            }

            Declaration::Module { value, .. } => (RefKind::Mixed, Weak::Inherit(*value)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        infer::weak::{ToWeak, Weak},
        RefKind, Type,
    };
    use lang::test::fixture as f;

    #[test]
    fn type_alias() {
        assert_eq!(
            f::a::type_("Foo", 0).to_weak(),
            (RefKind::Type, Weak::Inherit(0))
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            f::a::enum_("Foo", vec![(String::from("Bar"), vec![0, 1, 2])]).to_weak(),
            (
                RefKind::Mixed,
                Weak::Type(Type::Enumerated(vec![(String::from("Bar"), vec![0, 1, 2])]))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            f::a::const_("FOO", None, 0).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn constant_with_type() {
        assert_eq!(
            f::a::const_("FOO", Some(0), 1).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            f::a::func_("foo", vec![0, 1], None, 2).to_weak(),
            (RefKind::Value, Weak::Type(Type::Function(vec![0, 1], 2)))
        );
    }

    #[test]
    fn function_with_typedef() {
        assert_eq!(
            f::a::func_("foo", vec![0, 1], Some(2), 3).to_weak(),
            (RefKind::Value, Weak::Type(Type::Function(vec![0, 1], 2)))
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            f::a::view("Foo", vec![0, 1], 2).to_weak(),
            (RefKind::Value, Weak::Type(Type::View(vec![0, 1])))
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            f::a::module("foo", 0).to_weak(),
            (RefKind::Mixed, Weak::Inherit(0))
        );
    }
}
