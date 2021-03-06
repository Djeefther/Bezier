#ifndef BEZIER_H_
#define BEZIER_H_

#include <vector>

template<class PointType>
class Bezier {
	std::vector<PointType> control_points_;
	std::vector<std::vector<PointType>> auxiliar_points_;
	//auxiliar_points are n-1 lines, where the first line has n-1 points, the second has n-2
	//and so on until last line has just one element
public:
	class iterator;
	std::vector<PointType>& control_points() {
		return control_points_;
	}
	const std::vector<std::vector<PointType>>& auxiliar_points() const {
		return auxiliar_points_;
	}
	PointType point(double t) {
		resize_auxiliar_points();
		set_auxiliar_points_(t);
		return auxiliar_points_.back().front();
	}
	bool valid() {
		return control_points_.size() > 1;
	}
private:
	void resize_auxiliar_points() {
		int n = control_points_.size() - 1;

		if (auxiliar_points_.size() != n) {
			auxiliar_points_.resize(n);

			for (auto& i : auxiliar_points_) {
				i.resize(n);
				n--;
			}
		}
	}
	void set_auxiliar_points_(double t) {
		const std::vector<PointType> * last_line = &control_points_;
		for (auto& line : auxiliar_points_) {
			size_t index_point = 0;
			for (auto& point : line) {
				point = (last_line->operator[](index_point) * (1 - t)) +
					(last_line->operator[](index_point + 1) * t);
				index_point++;
			}

			last_line = &line;
		}
	}
};

#endif