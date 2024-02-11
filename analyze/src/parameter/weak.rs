use crate::infer::weak::{ToWeak, Weak, WeakRef};
use lang::{
    ast::{self, walk},
    types,
};

impl ToWeak for ast::Parameter<String, walk::NodeId, walk::NodeId> {
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
                } => Weak::Inherit(*x),

                Self { .. } => Weak::Infer,
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::infer::weak::{ToWeak, Weak};
    use kore::str;
    use lang::{
        ast::{self, walk::NodeId},
        types,
    };

    #[test]
    fn unknown_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, None).to_weak(),
            (types::RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn typedef_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(0)), None).to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn default_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, Some(NodeId(0))).to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn typedef_and_default_parameter() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(0)), Some(NodeId(1))).to_weak(),
            (types::RefKind::Value, Weak::Inherit(NodeId(0)))
        );
    }
}
