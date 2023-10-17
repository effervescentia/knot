use super::{RefKind, Type, Weak};
use crate::infer::weak::{ToWeak, WeakRef};
use lang::ast::KSX;

impl ToWeak for KSX<usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            KSX::Text(..) => (RefKind::Value, Weak::Type(Type::String)),

            KSX::Inline(id) => (RefKind::Value, Weak::Inherit(*id)),

            KSX::Fragment(..) => (RefKind::Value, Weak::Type(Type::Element)),

            KSX::ClosedElement(..) => (RefKind::Value, Weak::Type(Type::Element)),

            KSX::OpenElement(..) => (RefKind::Value, Weak::Type(Type::Element)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        infer::weak::{ToWeak, Weak},
        RefKind, Type,
    };
    use lang::ast::KSX;

    #[test]
    fn text() {
        assert_eq!(
            KSX::Text(String::from("foo")).to_weak(),
            (RefKind::Value, Weak::Type(Type::String))
        );
    }

    #[test]
    fn inline() {
        assert_eq!(KSX::Inline(0).to_weak(), (RefKind::Value, Weak::Inherit(0)));
    }

    #[test]
    fn fragment() {
        assert_eq!(
            KSX::Fragment(vec![0]).to_weak(),
            (RefKind::Value, Weak::Type(Type::Element))
        );
    }

    #[test]
    fn closed_element() {
        assert_eq!(
            KSX::ClosedElement(
                String::from("Foo"),
                vec![(String::from("bar"), None), (String::from("fizz"), Some(0))]
            )
            .to_weak(),
            (RefKind::Value, Weak::Type(Type::Element))
        );
    }

    #[test]
    fn open_element() {
        assert_eq!(
            KSX::OpenElement(
                String::from("Foo"),
                vec![(String::from("bar"), None), (String::from("fizz"), Some(0))],
                vec![1],
                String::from("Foo"),
            )
            .to_weak(),
            (RefKind::Value, Weak::Type(Type::Element))
        );
    }
}
