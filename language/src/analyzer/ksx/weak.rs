use super::{RefKind, Type, WeakType};
use crate::{
    analyzer::{infer::weak::ToWeak, WeakRef},
    parser::ksx::KSX,
};

impl ToWeak for KSX<usize, usize> {
    fn to_weak(&self) -> WeakRef {
        match self {
            KSX::Text(..) => (RefKind::Value, WeakType::Strong(Type::String)),

            KSX::Inline(id) => (RefKind::Value, WeakType::Reference(*id)),

            KSX::Fragment(..) => (RefKind::Value, WeakType::Strong(Type::Element)),

            KSX::ClosedElement(..) => (RefKind::Value, WeakType::Strong(Type::Element)),

            KSX::OpenElement(..) => (RefKind::Value, WeakType::Strong(Type::Element)),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{infer::weak::ToWeak, RefKind, Type, WeakType},
        parser::ksx::KSX,
    };

    #[test]
    fn text() {
        assert_eq!(
            KSX::Text(String::from("foo")).to_weak(),
            (RefKind::Value, WeakType::Strong(Type::String))
        );
    }

    #[test]
    fn inline() {
        assert_eq!(
            KSX::Inline(0).to_weak(),
            (RefKind::Value, WeakType::Reference(0))
        );
    }

    #[test]
    fn fragment() {
        assert_eq!(
            KSX::Fragment(vec![0]).to_weak(),
            (RefKind::Value, WeakType::Strong(Type::Element))
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
            (RefKind::Value, WeakType::Strong(Type::Element))
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
            (RefKind::Value, WeakType::Strong(Type::Element))
        );
    }
}
