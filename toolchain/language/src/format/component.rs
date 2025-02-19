use crate::ast;
use kore::format::{indented, PrefixEach};
use std::fmt::{Display, Formatter, Write};

pub trait IsInline {
    fn is_inline(&self) -> bool;
}

impl<Component, Expression, Attribute> IsInline
    for ast::Component<Component, Expression, Attribute>
{
    fn is_inline(&self) -> bool {
        matches!(self, Self::Text(_) | Self::Expression(_))
    }
}

impl IsInline for ast::shape::Component {
    fn is_inline(&self) -> bool {
        self.0.is_inline()
    }
}

impl<Expression> Display for ast::Attribute<Expression>
where
    Expression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Punned(name) => write!(f, "{name}"),

            Self::Explicit(name, value) => {
                write!(f, "{name}={value}")
            }
        }
    }
}

impl<Component, Expression, Attribute> Display for ast::Component<Component, Expression, Attribute>
where
    Component: Display + IsInline,
    Expression: Display,
    Attribute: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Text(x) => write!(f, "{x}"),

            Self::Expression(x) => write!(f, "{{{x}}}"),

            Self::Fragment(children) => write!(f, "<>{children}</>", children = Children(children)),

            Self::ClosedElement(tag, attributes) => write!(
                f,
                "<{tag}{attributes} />",
                attributes = PrefixEach(" ", attributes)
            ),

            Self::OpenElement {
                start_tag,
                attributes,
                children,
                end_tag,
            } => {
                write!(
                    f,
                    "<{start_tag}{attributes}>{children}</{end_tag}>",
                    attributes = PrefixEach(" ", attributes),
                    children = Children(children)
                )
            }
        }
    }
}

struct Children<'a, Component>(&'a Vec<Component>);

impl<Component> Display for Children<'_, Component>
where
    Component: Display + IsInline,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        fn format<Comp, F>(xs: &Vec<Comp>, mut f: F, all_inline: bool) -> std::fmt::Result
        where
            Comp: Display + IsInline,
            F: Write,
        {
            if all_inline {
                for x in xs {
                    write!(f, "{x}")?;
                }
            } else {
                let mut prev_inline = None;

                for x in xs {
                    let next_inline = x.is_inline();

                    match (prev_inline, next_inline) {
                        (_, false) | (Some(false), true) | (None, _) => {
                            prev_inline = Some(next_inline);
                            writeln!(f)?;
                        }
                        _ => (),
                    }

                    write!(f, "{x}")?;
                }

                writeln!(f)?;
            }

            Ok(())
        }

        if self.0.iter().all(IsInline::is_inline) {
            format(self.0, f, true)
        } else {
            format(self.0, indented(f), false)
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use kore::str;

    #[test]
    fn text() {
        assert_eq!(
            ast::shape::Component(ast::Component::Text(str!("foo"))).to_string(),
            "foo"
        );
    }

    #[test]
    fn inline_expression() {
        assert_eq!(
            ast::shape::Component(ast::Component::Expression(ast::shape::Expression(
                ast::Expression::Primitive(ast::Primitive::Nil)
            )))
            .to_string(),
            "{nil}"
        );
    }

    #[test]
    fn empty_fragment() {
        assert_eq!(
            ast::shape::Component(ast::Component::Fragment(vec![])).to_string(),
            "<></>"
        );
    }

    #[test]
    fn fragment_with_children() {
        assert_eq!(
            ast::shape::Component(ast::Component::Fragment(vec![ast::shape::Component(
                ast::Component::Text(str!("foo"))
            )]))
            .to_string(),
            "<>foo</>"
        );
    }

    #[test]
    fn closed_element_no_attributes() {
        assert_eq!(
            ast::shape::Component(ast::Component::ClosedElement(str!("foo"), vec![])).to_string(),
            "<foo />"
        );
    }

    #[test]
    fn closed_element_with_attributes() {
        assert_eq!(
            ast::shape::Component(ast::Component::ClosedElement(
                str!("foo"),
                vec![
                    ast::shape::Attribute(ast::Attribute::Punned(str!("fizz"))),
                    ast::shape::Attribute(ast::Attribute::Explicit(
                        str!("buzz"),
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                    )),
                ]
            ))
            .to_string(),
            "<foo fizz buzz=nil />"
        );
    }

    #[test]
    fn empty_open_element() {
        assert_eq!(
            ast::shape::Component(ast::Component::open_element(
                str!("foo"),
                vec![],
                vec![],
                str!("foo")
            ))
            .to_string(),
            "<foo></foo>"
        );
    }

    #[test]
    fn open_element_with_attributes() {
        assert_eq!(
            ast::shape::Component(ast::Component::open_element(
                str!("foo"),
                vec![
                    ast::shape::Attribute(ast::Attribute::Punned(str!("fizz"))),
                    ast::shape::Attribute(ast::Attribute::Explicit(
                        str!("buzz"),
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                    )),
                ],
                vec![],
                str!("foo"),
            ))
            .to_string(),
            "<foo fizz buzz=nil></foo>"
        );
    }

    #[test]
    fn open_element_with_children() {
        assert_eq!(
            ast::shape::Component(ast::Component::open_element(
                str!("foo"),
                vec![],
                vec![ast::shape::Component(ast::Component::Text(str!("bar")))],
                str!("foo"),
            ))
            .to_string(),
            "<foo>bar</foo>"
        );
    }

    #[test]
    fn element_children() {
        assert_eq!(
            ast::shape::Component(ast::Component::open_element(
                str!("foo"),
                vec![],
                vec![ast::shape::Component(ast::Component::ClosedElement(
                    str!("bar"),
                    vec![],
                ))],
                str!("foo"),
            ))
            .to_string(),
            "<foo>
  <bar />
</foo>"
        );
    }

    #[test]
    fn mixed_children() {
        assert_eq!(
            ast::shape::Component(ast::Component::open_element(
                str!("foo"),
                vec![],
                vec![
                    ast::shape::Component(ast::Component::Text(str!("hello "))),
                    ast::shape::Component(ast::Component::Expression(ast::shape::Expression(
                        ast::Expression::Identifier(str!("name"))
                    ))),
                    ast::shape::Component(ast::Component::Text(str!(", how are you doing?"))),
                    ast::shape::Component(ast::Component::ClosedElement(str!("Overview"), vec![],)),
                    ast::shape::Component(ast::Component::Expression(ast::shape::Expression(
                        ast::Expression::Identifier(str!("left"))
                    ))),
                    ast::shape::Component(ast::Component::Text(str!(" or "))),
                    ast::shape::Component(ast::Component::Expression(ast::shape::Expression(
                        ast::Expression::Identifier(str!("right"))
                    ))),
                    ast::shape::Component(ast::Component::ClosedElement(str!("Summary"), vec![],)),
                    ast::shape::Component(ast::Component::Text(str!("that's all folks!"))),
                ],
                str!("foo"),
            ))
            .to_string(),
            "<foo>
  hello {name}, how are you doing?
  <Overview />
  {left} or {right}
  <Summary />
  that's all folks!
</foo>"
        );
    }
}
