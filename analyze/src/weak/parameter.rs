use crate::{
    ast,
    data::ScopedType,
    weak::{ToWeak, WeakRef},
};
use lang::{types, NodeId};

impl ToWeak for ast::Parameter<String, NodeId, NodeId> {
    fn to_weak(&self) -> WeakRef {
        (
            types::RefKind::Value,
            match self {
                Self {
                    value_type: Some(x),
                    ..
                }
                | Self {
                    default_value: Some(x),
                    ..
                } => Some(ScopedType::Inherit(*x)),

                Self { .. } => None,
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{ast, data::ScopedType, weak::ToWeak};
    use kore::str;
    use lang::{types, NodeId};

    #[test]
    fn unknown_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, None).to_weak(),
            (types::RefKind::Value, None)
        );
    }

    #[test]
    fn typedef_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(0)), None).to_weak(),
            (types::RefKind::Value, Some(ScopedType::Inherit(NodeId(0))))
        );
    }

    #[test]
    fn default_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, Some(NodeId(0))).to_weak(),
            (types::RefKind::Value, Some(ScopedType::Inherit(NodeId(0))))
        );
    }

    #[test]
    fn typedef_and_default_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(0)), Some(NodeId(1))).to_weak(),
            (types::RefKind::Value, Some(ScopedType::Inherit(NodeId(0))))
        );
    }
}
