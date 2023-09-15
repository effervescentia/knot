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
