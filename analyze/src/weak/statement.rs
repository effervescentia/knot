use crate::{
    ast::{self, walk},
    weak::{ToWeak, Weak, WeakRef},
};
use lang::types;

impl ToWeak for ast::Statement<walk::NodeId> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::Expression(id) => (types::RefKind::Value, Weak::Inherit(*id)),

            Self::Variable(..) => (types::RefKind::Value, Weak::Type(types::Type::Nil)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast::{self, walk::NodeId},
        weak::{ToWeak, Weak},
    };
    use kore::str;
    use lang::types;

    #[test]
    fn expression() {
        assert_eq!(
            ast::Statement::Expression(NodeId(0)).to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            ast::Statement::Variable(str!("foo"), NodeId(0)).to_weak(),
            (types::RefKind::Value, Weak::Type(types::Type::Nil))
        );
    }
}
