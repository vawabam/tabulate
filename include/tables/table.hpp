#include <iostream>
#include <vector>
#include <string>
#include <tables/row.hpp>
#include <tables/termcolor.hpp>
#include <tables/font_style.hpp>

namespace tables {

class Table {
public:

  void add_row(const std::vector<std::string>& cells) {
    Row row;
    for (auto& cell : cells) {
      row.add_cell(Cell(cell));
    }
    rows_.push_back(row);
  }

  Row& operator[](size_t index) {
    return rows_[index];
  }

  Format& format() {
    return format_;
  }

private:
  friend std::ostream& operator<<(std::ostream &os, const Table& table);
  
  void print(std::ostream& stream = std::cout) const {    
    for (size_t i = 0; i < rows_.size(); ++i) {
      auto& row = rows_[i];
      auto row_format = row.format_;
      auto row_height = get_row_height(i);
      const auto& cells = row.cells();
      
      // Header row
      for (size_t j = 0; j < cells.size(); ++j) {
	print_cell_header(stream, i, j);
      }
      stream << "\n";

      // Padding top
      Format format = get_format(row_format, {});
      for (size_t k = 0; k < format.padding_top_; ++k) {
	print_padding_row(stream, i);
	stream << "\n";
      }

      // Row contents
      for (size_t k = 0; k < row_height; ++k) {
	std::vector<std::string> sub_row_contents;
	std::vector<size_t> column_widths;
	for (size_t l = 0; l < cells.size(); ++l) {
	  auto column_width = get_column_width(l);
	  auto cell_contents = cells[l].data();
	  auto pos = k * column_width;
	  auto size = cell_contents.size();	  
	  if (pos < size) {
	    auto remaining = (size - pos);
	    sub_row_contents.push_back
	      (cell_contents.substr(pos,
				    std::min(remaining,
					     column_width)));
	  } else {
	    sub_row_contents.push_back("");
	  }
	  column_widths.push_back(column_width);
	}
	print_content_row(stream, i, sub_row_contents, column_widths);
      }
      
      // Padding bottom
      for (size_t k = 0; k < format.padding_bottom_; ++k) {
	print_padding_row(stream, i);
	stream << "\n";
      }

      // Footer row      
      if (i + 1 == rows_.size()) {
	for (size_t j = 0; j < cells.size(); ++j) {
	  print_cell_footer(stream, i, j);
	}
      }
    }
  }

  void apply_font_style(std::ostream& stream, Format format) const {
    reset_style(stream);
    auto font_style = format.font_style_;
    for (auto& style : font_style) {
      switch(style) {
      case FontStyle::bold:
	stream << termcolor::bold;
	break;
      case FontStyle::dark:
	stream << termcolor::dark;
	break;
      case FontStyle::italic:
	stream << termcolor::italic;
	break;	
      case FontStyle::underline:
	stream << termcolor::underline;
	break;
      case FontStyle::blink:
	stream << termcolor::blink;
	break;
      case FontStyle::reverse:
	stream << termcolor::reverse;
	break;
      case FontStyle::concealed:
	stream << termcolor::concealed;
	break;
      case FontStyle::crossed:
	stream << termcolor::crossed;
	break;	
      default:
	break;
      }
    }

    auto color = format.color_;
    if (color.has_value()) {
      switch (color.value()) {
      case Color::grey:
	std::cout << termcolor::grey;
	break;
      case Color::red:
	std::cout << termcolor::red;
	break;
      case Color::green:
	std::cout << termcolor::green;
	break;
      case Color::yellow:
	std::cout << termcolor::yellow;
	break;
      case Color::blue:
	std::cout << termcolor::blue;
	break;
      case Color::magenta:
	std::cout << termcolor::magenta;
	break;
      case Color::cyan:
	std::cout << termcolor::cyan;
	break;
      case Color::white:
	std::cout << termcolor::white;
	break;
      }
    }

    auto background_color = format.background_color_;
    if (background_color.has_value()) {
      switch (background_color.value()) {
      case Color::grey:
	std::cout << termcolor::on_grey;
	break;
      case Color::red:
	std::cout << termcolor::on_red;
	break;
      case Color::green:
	std::cout << termcolor::on_green;
	break;
      case Color::yellow:
	std::cout << termcolor::on_yellow;
	break;
      case Color::blue:
	std::cout << termcolor::on_blue;
	break;
      case Color::magenta:
	std::cout << termcolor::on_magenta;
	break;
      case Color::cyan:
	std::cout << termcolor::on_cyan;
	break;
      case Color::white:
	std::cout << termcolor::on_white;
	break;
      }
    }    
    
  }

  void reset_style(std::ostream& stream) const {
    stream << termcolor::reset;
  }

  size_t get_column_width(size_t index) const {
    size_t result{0};
    for (auto& row : rows_) {
      if (index < row.size()) {
	auto cell = row.get_cell(index);
	size_t cell_width = cell.has_value() ? cell.value().size() : 0;
	if (format_.width_.has_value()) {
	  result = format_.width_.value();	  
	} else {
	  result = std::max(result, cell_width);
	}
      }
    }
    return result;
  }

  size_t get_row_height(size_t index) const {
    size_t result{1};
    if (index < rows_.size()) {
      result = std::max(result, rows_[index].height());
    }
    return result;
  }

  void print_padding_row(std::ostream& stream, size_t row_index) const {
    Format format;
    auto row = rows_[row_index];
    auto row_format = row.format_;
    for (size_t col_index = 0; col_index < row.size(); ++col_index) {
      auto cell = row[col_index];
      auto cell_format = cell.format_;

      format = get_format(row_format, cell_format);
      
      auto width = get_column_width(col_index);

      // add padding to width
      width += format.padding_left_;
      width += format.padding_right_;

      if (col_index == 0)
	stream << format.border_left_;
      else
	stream << format.column_separator_;
      
      size_t i = 0;
      while(i < width) {
	stream << " ";
	++i;
      }
    }
    stream << format.border_right_;
  }

  void print_content_row(std::ostream& stream, size_t row_index,
			 std::vector<std::string> row_contents, std::vector<size_t> column_widths) const {
    Format format;
    auto row = rows_[row_index];
    auto row_format = row.format_;
    for (size_t i = 0; i < row_contents.size(); ++i) {
      auto cell = row[i];
      auto cell_format = cell.format_;

      format = get_format(row_format, cell_format);
      
      auto cell_content = row_contents[i];

      if (i == 0)
	stream << format.border_left_;
      else
	stream << format.column_separator_;      

      for (size_t j = 0; j < format.padding_left_; ++j) {
	stream << " ";     
      }

      apply_font_style(stream, format);
      switch(format.font_align_) {
      case FontAlign::left:
	print_content_left_aligned(stream, cell_content, column_widths[i]);
	break;
      case FontAlign::center:
	print_content_center_aligned(stream, cell_content, column_widths[i]);
	break;	
      case FontAlign::right:
	print_content_right_aligned(stream, cell_content, column_widths[i]);
	break;
      }
      reset_style(stream);
      
      for (size_t j = 0; j < format.padding_right_; ++j) {
	stream << " ";
      }
    }
    stream << format.border_right_;
    std::cout << "\n";
  }

  void print_content_left_aligned(std::ostream& stream, std::string cell_content, size_t column_width) const {
    stream << cell_content;
    auto content_width = cell_content.size();
    if (content_width < column_width) {
      for (size_t j = 0; j < (column_width - content_width); ++j) {
	stream << " ";
      }
    }
  }

  void print_content_center_aligned(std::ostream& stream, std::string cell_content, size_t column_width) const {
    auto content_width = cell_content.size();
    auto num_spaces = column_width - content_width;
    if (num_spaces % 2 == 0) {
      // Even spacing on either side
      for (size_t j = 0; j < num_spaces / 2; ++j)
	stream << " ";
      stream << cell_content;
      for (size_t j = 0; j < num_spaces / 2; ++j)
	stream << " ";
    } else {
      auto num_spaces_before = num_spaces / 2 + 1;
      for (size_t j = 0; j < num_spaces_before; ++j)
	stream << " ";
      stream << cell_content;
      for (size_t j = 0; j < num_spaces - num_spaces_before; ++j)
	stream << " ";            
    }
  }  

  void print_content_right_aligned(std::ostream& stream, std::string cell_content, size_t column_width) const {
    auto content_width = cell_content.size();
    if (content_width < column_width) {
      for (size_t j = 0; j < (column_width - content_width); ++j) {
	stream << " ";
      }
    }    
    stream << cell_content;
  }

  void print_cell_header(std::ostream& stream, size_t row_index, size_t col_index) const {
    auto row = rows_[row_index];
    auto row_format = row.format_;
    auto cell_format = row[col_index].format_;

    auto width = get_column_width(col_index);

    Format format = get_format(row_format, cell_format);

    // add padding to width
    width += format.padding_left_;
    width += format.padding_right_;

    if (col_index == 0)
      stream << format.corners_;

    size_t i = 0;
    while(i < width) {
      stream << format.border_top_;
      ++i;
    }

    stream << format.corners_;
  }

  void print_cell_footer(std::ostream& stream, size_t row_index, size_t col_index) const {
    auto row = rows_[row_index];
    auto row_format = row.format_;
    auto cell_format = row[col_index].format_;

    auto width = get_column_width(col_index);

    Format format = get_format(row_format, cell_format);

    // add padding to width
    width += format.padding_left_;
    width += format.padding_right_;

    if (col_index == 0)
      stream << format.corners_;

    size_t i = 0;
    while(i < width) {
      stream << format.border_bottom_;
      ++i;
    }

    stream << format.corners_;
  }

  Format get_format(std::optional<Format> row_format, std::optional<Format> cell_format) const {
    Format result;
    // Check for cell-level formatting override
    // This format takes preference
    if (cell_format.has_value()) {
      result = cell_format.value();
    }
    // No cell formatting override
    // Check for row-level formatting
    else if (row_format.has_value()) {
      result = row_format.value();
    }
    // No cell or row formatting overrides
    // Use table-level formatting
    else {
      result = format_;
    }
    return result;
  }
  
  std::vector<Row> rows_;
  Format format_;
};

std::ostream& operator<<(std::ostream &os, const Table& table) {
  table.print(os);
  return os;
}  

}
