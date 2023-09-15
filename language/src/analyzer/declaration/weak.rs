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
                WeakType::Strong(Type::Enumerated(variants.clone())),
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
        analyzer::{infer::weak::ToWeak, RefKind, Type, WeakType},
        test::fixture as f,
    };

    #[test]
    fn type_alias() {
        assert_eq!(
            f::a::type_("Foo", 0).to_weak(),
            (RefKind::Type, WeakType::Reference(0))
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            f::a::enum_("Foo", vec![(String::from("Bar"), vec![0, 1, 2])]).to_weak(),
            (
                RefKind::Value,
                WeakType::Strong(Type::Enumerated(vec![(String::from("Bar"), vec![0, 1, 2])]))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            f::a::const_("FOO", None, 0).to_weak(),
            (RefKind::Value, WeakType::Reference(0))
        );
    }

    #[test]
    fn constant_with_type() {
        assert_eq!(
            f::a::const_("FOO", Some(0), 1).to_weak(),
            (RefKind::Value, WeakType::Reference(0))
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            f::a::func_("foo", vec![], None, 0).to_weak(),
            (RefKind::Value, WeakType::Any)
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            f::a::view("Foo", vec![], 0).to_weak(),
            (RefKind::Value, WeakType::Any)
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            f::a::mod_("foo", 0).to_weak(),
            (RefKind::Value, WeakType::Any)
        );
    }
}
