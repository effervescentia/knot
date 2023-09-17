use crate::{
    analyzer::{infer::weak::ToWeak, RefKind, WeakRef, WeakType},
    parser::declaration::parameter::Parameter,
};

impl ToWeak for Parameter<usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Parameter {
                value_type: Some(x),
                ..
            }
            | Parameter {
                default_value: Some(x),
                ..
            } => (RefKind::Value, WeakType::Reference(*x)),

            Parameter { .. } => (RefKind::Value, WeakType::Any),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{infer::weak::ToWeak, RefKind, WeakType},
        parser::declaration::parameter::Parameter,
    };

    #[test]
    fn unknown_parameter() {
        assert_eq!(
            Parameter {
                name: String::from("foo"),
                value_type: None,
                default_value: None
            }
            .to_weak(),
            (RefKind::Value, WeakType::Any)
        );
    }

    #[test]
    fn typedef_parameter() {
        assert_eq!(
            Parameter {
                name: String::from("foo"),
                value_type: Some(0),
                default_value: None
            }
            .to_weak(),
            (RefKind::Value, WeakType::Reference(0))
        );
    }

    #[test]
    fn default_parameter() {
        assert_eq!(
            Parameter {
                name: String::from("foo"),
                value_type: None,
                default_value: Some(0)
            }
            .to_weak(),
            (RefKind::Value, WeakType::Reference(0))
        );
    }

    #[test]
    fn typedef_and_default_parameter() {
        assert_eq!(
            Parameter {
                name: String::from("foo"),
                value_type: Some(0),
                default_value: Some(1)
            }
            .to_weak(),
            (RefKind::Value, WeakType::Reference(0))
        );
    }
}
