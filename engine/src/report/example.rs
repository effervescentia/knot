use kore::{
    color::{Colorize, Highlight},
    format::Indented,
    str,
};

pub fn local_variables(name: &str) -> (String, String) {
    (
        str!("Local Variables"),
        format!(
            "In this example the {} keyword is used to declare a local variable named {}.

{}",
            "let".highlight(),
            name.highlight(),
            Indented(format!("{} {} = 123;", "let".highlight(), name.highlight()).dimmed())
        ),
    )
}

pub fn constants<T>(name: T) -> (String, String)
where
    T: AsRef<str>,
{
    (
        str!("Constants"),
        format!(
            "In this example the {} keyword is used to declare a constant named {}.

{}",
            "const".highlight(),
            name.as_ref().highlight(),
            Indented(
                format!(
                    "{} {} = 123;",
                    "const".highlight(),
                    name.as_ref().highlight()
                )
                .dimmed()
            )
        ),
    )
}

pub fn enumerated_type_variants<T1, T2>(first: T1, second: T2) -> (String, String)
where
    T1: AsRef<str>,
    T2: AsRef<str>,
{
    (
        str!("Enumerated Type Variants"),
        format!(
            "In this example the enumerated type has variants named {} and {} respectively.

{}",
            first.as_ref().highlight(),
            second.as_ref().highlight(),
            Indented(
                format!(
                    "enum Example =
  {}
  {}
;",
                    format!("| {}", first.as_ref()).highlight(),
                    format!("| {}", second.as_ref()).highlight(),
                )
                .dimmed()
            )
        ),
    )
}

pub fn object_properties<T1, T2>(first: T1, second: T2) -> (String, String)
where
    T1: AsRef<str>,
    T2: AsRef<str>,
{
    (
        str!("Properties"),
        format!(
            "In this example properties named {} and {} are declared.
All property names must be unique.

{}",
            first.as_ref().highlight(),
            second.as_ref().highlight(),
            Indented(
                format!(
                    "type Example = {{
  {}: integer,
  {}: integer,
}};",
                    first.as_ref().highlight(),
                    second.as_ref().highlight()
                )
                .dimmed()
            )
        ),
    )
}

pub fn parameter_types() -> (String, String) {
    (
        str!("Parameter Types"),
        format!(
            "In the example below both parameters are declared to have type {}.

{}",
            "integer".highlight(),
            Indented(
                format!(
                    "func add(left{}, right{}) -> left + right;",
                    ": integer".highlight(),
                    ": integer".highlight(),
                )
                .dimmed()
            )
        ),
    )
}

pub fn open_component(name: &str) -> (String, String) {
    (
        str!("Open Component"),
        format!(
            "In the example below a component {} is rendered with raw text as a child.

{}",
            name.highlight(),
            Indented(
                format!(
                    "{}Hello, World!{}",
                    format!("<{name}>").highlight(),
                    format!("</{name}>").highlight(),
                )
                .dimmed()
            )
        ),
    )
}

pub fn component_attributes<T1, T2>(first: T1, second: T2) -> (String, String)
where
    T1: AsRef<str>,
    T2: AsRef<str>,
{
    (
        str!("Component Attributes"),
        format!(
            "In the example below a component is rendered with attributes {} and {}.

{}",
            first.as_ref().highlight(),
            second.as_ref().highlight(),
            Indented(
                format!(
                    "<Example {}=123 {}=true />",
                    first.as_ref().highlight(),
                    second.as_ref().highlight()
                )
                .dimmed()
            )
        ),
    )
}
