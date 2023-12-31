use super::{RefKind, Type, Weak};
use crate::infer::weak::{ToWeak, WeakRef};
use lang::ast::Statement;

impl ToWeak for Statement<usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::Expression(id) => (RefKind::Value, Weak::Inherit(*id)),

            Self::Variable(..) => (RefKind::Value, Weak::Type(Type::Nil)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        infer::weak::{ToWeak, Weak},
        RefKind, Type,
    };
    use kore::str;
    use lang::ast::Statement;

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
            Statement::Variable(str!("foo"), 0).to_weak(),
            (RefKind::Value, Weak::Type(Type::Nil))
        );
    }
}
