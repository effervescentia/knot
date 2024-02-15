use crate::{
    ast,
    data::ScopedType,
    weak::{ToWeak, WeakRef},
};
use lang::{types, NodeId};

impl ToWeak for ast::Component<NodeId, NodeId> {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::Text(..) => (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::String)),
            ),

            Self::Expression(id) => (types::RefKind::Value, Some(ScopedType::Inherit(*id))),

            Self::Fragment(..) | Self::ClosedElement(..) | Self::OpenElement { .. } => (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::Element)),
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
    fn text() {
        assert_eq!(
            ast::Component::Text(str!("foo")).to_weak(),
            (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::String))
            )
        );
    }

    #[test]
    fn expression() {
        assert_eq!(
            ast::Component::Expression(NodeId(0)).to_weak(),
            (types::RefKind::Value, Some(ScopedType::Inherit(NodeId(0))))
        );
    }

    #[test]
    fn fragment() {
        assert_eq!(
            ast::Component::Fragment(vec![NodeId(0)]).to_weak(),
            (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::Element))
            )
        );
    }

    #[test]
    fn closed_element() {
        assert_eq!(
            ast::Component::ClosedElement(
                str!("Foo"),
                vec![(str!("bar"), None), (str!("fizz"), Some(NodeId(0)))]
            )
            .to_weak(),
            (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::Element))
            )
        );
    }

    #[test]
    fn open_element() {
        assert_eq!(
            ast::Component::open_element(
                str!("Foo"),
                vec![(str!("bar"), None), (str!("fizz"), Some(NodeId(0)))],
                vec![NodeId(1)],
                str!("Foo"),
            )
            .to_weak(),
            (
                types::RefKind::Value,
                Some(ScopedType::Type(types::Type::Element))
            )
        );
    }
}
