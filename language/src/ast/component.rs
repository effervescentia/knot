use crate::walk::{ProgramVisitor, Walk, WalkEach};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Attribute<Expression> {
    Punned(String),
    Explicit(String, Expression),
}

impl<Expression> Attribute<Expression> {
    pub fn name(&self) -> &str {
        match self {
            Self::Punned(name) | Self::Explicit(name, _) => name,
        }
    }
}

impl<Visitor, Context, Expression> Walk<Visitor> for (Attribute<Expression>, Context)
where
    Visitor: ProgramVisitor<Context = Context>,
    Expression: Walk<Visitor, Output = Visitor::Expression>,
{
    type Output = Visitor::Attribute;

    fn walk(self, v: Visitor) -> (Self::Output, Visitor) {
        let (value, ctx) = self;

        match value {
            Attribute::Punned(name) => v.attribute(Attribute::Punned(name), ctx),

            Attribute::Explicit(name, x) => {
                let (x, v) = x.walk(v);

                v.attribute(Attribute::Explicit(name, x), ctx)
            }
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub enum Component<Component_, Expression, Attribute> {
    Text(String),
    Expression(Expression),
    Fragment(Vec<Component_>),
    ClosedElement(String, Vec<Attribute>),
    OpenElement {
        start_tag: String,
        attributes: Vec<Attribute>,
        children: Vec<Component_>,
        end_tag: String,
    },
}

impl<Component_, Expression, Attribute> Component<Component_, Expression, Attribute> {
    pub const fn open_element(
        start_tag: String,
        attributes: Vec<Attribute>,
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

impl<Visitor, Context, Component_, Expression, Attribute> Walk<Visitor>
    for (Component<Component_, Expression, Attribute>, Context)
where
    Visitor: ProgramVisitor<Context = Context>,
    Component_: Walk<Visitor, Output = Visitor::Component>,
    Expression: Walk<Visitor, Output = Visitor::Expression>,
    Attribute: Walk<Visitor, Output = Visitor::Attribute>,
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
