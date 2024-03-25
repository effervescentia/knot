use crate::ast;
use lang::{
    types::{RefKind, Type},
    NodeId,
};

impl super::ToWeak for ast::Statement<NodeId> {
    fn to_weak(&self) -> super::Ref {
        match self {
            Self::Expression(id) => (RefKind::Value, super::Type::Inherit(*id)),

            Self::Variable(..) => (RefKind::Value, super::Type::Local(Type::Nil)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        weak::{self, ToWeak},
    };
    use kore::str;
    use lang::{
        types::{RefKind, Type},
        NodeId,
    };

    #[test]
    fn expression() {
        assert_eq!(
            ast::Statement::Expression(NodeId(0)).to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            ast::Statement::Variable(str!("foo"), NodeId(0)).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Nil))
        );
    }
}
