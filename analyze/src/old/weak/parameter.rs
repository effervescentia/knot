use crate::ast;
use lang::{types::RefKind, NodeId};

impl super::ToWeak for ast::Parameter<String, NodeId, NodeId> {
    fn to_weak(&self) -> super::Ref {
        (
            RefKind::Value,
            match self {
                Self {
                    value_type: Some(x),
                    ..
                } => super::Type::inherit_from_type(*x),

                Self {
                    default_value: Some(x),
                    ..
                } => super::Type::Inherit(*x),

                Self { .. } => super::Type::Infer(super::Inference::Parameter),
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        weak::{self, ToWeak},
    };
    use kore::str;
    use lang::{types::RefKind, NodeId};

    #[test]
    fn unknown_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, None).to_weak(),
            (
                RefKind::Value,
                weak::Type::Infer(weak::Inference::Parameter)
            )
        );
    }

    #[test]
    fn typedef_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(0)), None).to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn default_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, Some(NodeId(0))).to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn typedef_and_default_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(0)), Some(NodeId(1))).to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }
}
