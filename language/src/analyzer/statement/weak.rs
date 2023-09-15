use super::{RefKind, Type, WeakType};
use crate::{
    analyzer::{infer::weak::ToWeak, WeakRef},
    parser::statement::Statement,
};

impl ToWeak for Statement<usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Statement::Effect(id) => (RefKind::Value, WeakType::Reference(*id)),

            Statement::Variable(..) => (RefKind::Value, WeakType::Strong(Type::Nil)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{infer::weak::ToWeak, RefKind, Type, WeakType},
        parser::statement::Statement,
    };

    #[test]
    fn effect() {
        assert_eq!(
            Statement::Effect(0).to_weak(),
            (RefKind::Value, WeakType::Reference(0))
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            Statement::Variable(String::from("foo"), 0).to_weak(),
            (RefKind::Value, WeakType::Strong(Type::Nil))
        );
    }
}
