use super::walk::{self, WalkEach};
use crate::Range;
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Component<Component_, Expression> {
    Text(String),
    Expression(Expression),
    Fragment(Vec<Component_>),
    ClosedElement(String, Vec<(String, Option<Expression>)>),
    OpenElement {
        start_tag: String,
        attributes: Vec<(String, Option<Expression>)>,
        children: Vec<Component_>,
        end_tag: String,
    },
}

impl<Component_, Expression> Component<Component_, Expression> {
    pub const fn open_element(
        start_tag: String,
        attributes: Vec<(String, Option<Expression>)>,
        children: Vec<Component_>,
        end_tag: String,
    ) -> Self {
        Self::OpenElement {
            start_tag,
            attributes,
            children,
            end_tag,
        }
    }
}

impl<Visitor, Component_, Expression> walk::Walk<Visitor, Range>
    for walk::Span<Component<Component_, Expression>>
where
    Visitor: walk::Visit<Range>,
    Component_: walk::Walk<Visitor, Range, Output = Visitor::Component>,
    Expression: walk::Walk<Visitor, Range, Output = Visitor::Expression>,
{
    type Output = Visitor::Component;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let Self(value, range) = self;

        match value {
            Component::Text(x) => v.component(Component::Text(x), range),

            Component::Expression(x) => {
                let (x, v) = x.walk(v);

                v.component(Component::Expression(x), range)
            }

            Component::Fragment(xs) => {
                let (xs, v) = xs.walk(v);

                v.component(Component::Fragment(xs), range)
            }

            Component::ClosedElement(tag, attributes) => {
                let (attributes, v) = attributes.walk(v);

                v.component(Component::ClosedElement(tag, attributes), range)
            }

            Component::OpenElement {
                start_tag,
                attributes,
                children,
                end_tag,
            } => {
                let ((attributes, children), v) = (attributes, children).walk_each(v);

                v.component(
                    Component::OpenElement {
                        start_tag,
                        attributes,
                        children,
                        end_tag,
                    },
                    range,
                )
            }
        }
    }
}
