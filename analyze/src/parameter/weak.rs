use crate::{
    infer::weak::{ToWeak, Weak, WeakRef},
    RefKind,
};
use lang::ast::Parameter;

impl ToWeak for Parameter<usize, usize> {
    fn to_weak(&self) -> WeakRef {
        (
            RefKind::Value,
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
    use crate::{
        infer::weak::{ToWeak, Weak},
        RefKind,
    };
    use kore::str;
    use lang::ast::Parameter;

    #[test]
    fn unknown_parameter() {
        assert_eq!(
            Parameter::new(str!("foo"), None, None).to_weak(),
            (RefKind::Value, Weak::Infer)
        );
    }

    #[test]
    fn typedef_parameter() {
        assert_eq!(
            Parameter::new(str!("foo"), Some(0), None).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn default_parameter() {
        assert_eq!(
            Parameter::new(str!("foo"), None, Some(0)).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn typedef_and_default_parameter() {
        assert_eq!(
            Parameter::new(str!("foo"), Some(0), Some(1)).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }
}
