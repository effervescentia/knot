use super::{RefKind, Type, Weak};
use crate::{
    analyzer::infer::weak::{ToWeak, WeakRef},
    ast::statement::Statement,
};

impl ToWeak for Statement<usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Statement::Expression(id) => (RefKind::Value, Weak::Inherit(*id)),

            Statement::Variable(..) => (RefKind::Value, Weak::Type(Type::Nil)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{
            infer::weak::{ToWeak, Weak},
            RefKind, Type,
        },
        ast::statement::Statement,
    };

    #[test]
    fn expression() {
        assert_eq!(
            Statement::Expression(0).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            Statement::Variable(String::from("foo"), 0).to_weak(),
            (RefKind::Value, Weak::Type(Type::Nil))
        );
    }
}
