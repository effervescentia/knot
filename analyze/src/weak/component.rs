use crate::ast;
use lang::{
    types::{RefKind, Type},
    NodeId,
};

impl super::ToWeak for ast::Component<NodeId, NodeId> {
    fn to_weak(&self) -> super::Ref {
        match self {
            Self::Text(..) => (RefKind::Value, super::Type::Local(Type::String)),

            Self::Expression(id) => (RefKind::Value, super::Type::Inherit(*id)),

            Self::Fragment(..) | Self::ClosedElement(..) | Self::OpenElement { .. } => {
                (RefKind::Value, super::Type::Local(Type::Element))
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        weak::{self, ToWeak},
    };
    use kore::str;
    use lang::{
        types::{RefKind, Type},
        NodeId,
    };

    #[test]
    fn text() {
        assert_eq!(
            ast::Component::Text(str!("foo")).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::String))
        );
    }

    #[test]
    fn expression() {
        assert_eq!(
            ast::Component::Expression(NodeId(0)).to_weak(),
            (RefKind::Value, weak::Type::Inherit(NodeId(0)))
        );
    }

    #[test]
    fn fragment() {
        assert_eq!(
            ast::Component::Fragment(vec![NodeId(0)]).to_weak(),
            (RefKind::Value, weak::Type::Local(Type::Element))
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
            (RefKind::Value, weak::Type::Local(Type::Element))
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
            (RefKind::Value, weak::Type::Local(Type::Element))
        );
    }
}
