use crate::{ast, data::ScopedType};
use lang::{types, NodeId};

impl super::ToWeak for ast::Statement<NodeId> {
    fn to_weak(&self) -> super::Ref {
        match self {
            Self::Expression(id) => (types::RefKind::Value, Some(ScopedType::Inherit(*id))),

            Self::Variable(..) => (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::Nil)),
            ),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{ast, data::ScopedType, weak::ToWeak};
    use kore::str;
    use lang::{types, NodeId};

    #[test]
    fn expression() {
        assert_eq!(
            ast::Statement::Expression(NodeId(0)).to_weak(),
            (types::RefKind::Value, Some(ScopedType::Inherit(NodeId(0))))
        );
    }

    #[test]
    fn variable() {
        assert_eq!(
            ast::Statement::Variable(str!("foo"), NodeId(0)).to_weak(),
            (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::Nil))
            )
        );
    }
}
