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

impl<Visitor, Meta, Component_, Expression> walk::Walk<Visitor, (Range, Meta)>
    for walk::Span<Component<Component_, Expression>, Meta>
where
    Visitor: walk::Visit<(Range, Meta)>,
    Component_: walk::Walk<Visitor, (Range, Meta), Output = Visitor::Component>,
    Expression: walk::Walk<Visitor, (Range, Meta), Output = Visitor::Expression>,
{
    type Output = Visitor::Component;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            Component::Text(x) => v.component(Component::Text(x), ctx),

            Component::Expression(x) => {
                let (x, v) = x.walk(v);

                v.component(Component::Expression(x), ctx)
            }

            Component::Fragment(xs) => {
                let (xs, v) = xs.walk(v);

                v.component(Component::Fragment(xs), ctx)
            }

            Component::ClosedElement(tag, attributes) => {
                let (attributes, v) = attributes.walk(v);

                v.component(Component::ClosedElement(tag, attributes), ctx)
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
                    ctx,
                )
            }
        }
    }
}
