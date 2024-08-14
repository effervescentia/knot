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

pub fn object_properties(first: &str, second: &str) -> (String, String) {
    (
        str!("Properties"),
        format!(
            "In this example properties named {} and {} are declared.

{}",
            first.highlight(),
            second.highlight(),
            Indented(
                format!(
                    "type Example = {{
  {}: integer,
  {}: integer,
}};",
                    first.highlight(),
                    second.highlight()
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
