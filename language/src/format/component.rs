use crate::ast;
use kore::format::indented;
use std::fmt::{Display, Formatter, Write};

pub trait IsInline {
    fn is_inline(&self) -> bool;
}

impl<Component_, Expression> IsInline for ast::Component<Component_, Expression> {
    fn is_inline(&self) -> bool {
        matches!(self, Self::Text(_) | Self::Expression(_))
    }
}

impl IsInline for ast::shape::Component {
    fn is_inline(&self) -> bool {
        self.0.is_inline()
    }
}

impl<Component, Expression> Display for ast::Component<Component, Expression>
where
    Component: Display + IsInline,
    Expression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            Self::Text(x) => write!(f, "{x}"),

            Self::Expression(x) => write!(f, "{{{x}}}"),

            Self::Fragment(children) => write!(f, "<>{children}</>", children = Children(children)),

            Self::ClosedElement(tag, attributes) => write!(
                f,
                "<{tag}{attributes} />",
                attributes = Attributes(attributes)
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
                    attributes = Attributes(attributes),
                    children = Children(children)
                )
            }
        }
    }
}

struct Attributes<'a, T>(&'a Vec<(String, Option<T>)>)
where
    T: Display;

impl<'a, T> Display for Attributes<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if self.0.is_empty() {
            Ok(())
        } else {
            self.0.iter().try_fold((), |_, (key, value)| {
                write!(f, " {key}")?;
                if let Some(value) = value {
                    write!(f, "={value}")
                } else {
                    Ok(())
                }
            })
        }
    }
}

struct Children<'a, Component>(&'a Vec<Component>);

impl<'a, Component> Display for Children<'a, Component>
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
                    (str!("fizz"), None),
                    (
                        str!("buzz"),
                        Some(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        )))
                    ),
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
                    (str!("fizz"), None),
                    (
                        str!("buzz"),
                        Some(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        )))
                    ),
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
