use crate::{
    analyzer::{infer::weak::ToWeak, RefKind, Weak, WeakRef},
    ast::parameter::Parameter,
};

impl ToWeak for Parameter<usize, usize> {
    fn to_weak(&self) -> WeakRef {
        (
            RefKind::Value,
            match self {
                Parameter {
                    value_type: Some(x),
                    ..
                }
                | Parameter {
                    default_value: Some(x),
                    ..
                } => Weak::Inherit(*x),

                Parameter { .. } => Weak::Unknown,
            },
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{infer::weak::ToWeak, RefKind, Weak},
        ast::parameter::Parameter,
    };

    #[test]
    fn unknown_parameter() {
        assert_eq!(
            Parameter::new(String::from("foo"), None, None).to_weak(),
            (RefKind::Value, Weak::Unknown)
        );
    }

    #[test]
    fn typedef_parameter() {
        assert_eq!(
            Parameter::new(String::from("foo"), Some(0), None).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn default_parameter() {
        assert_eq!(
            Parameter::new(String::from("foo"), None, Some(0)).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }

    #[test]
    fn typedef_and_default_parameter() {
        assert_eq!(
            Parameter::new(String::from("foo"), Some(0), Some(1)).to_weak(),
            (RefKind::Value, Weak::Inherit(0))
        );
    }
}
