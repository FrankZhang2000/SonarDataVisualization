#include "classifier.h"

GridElement::GridElement() {
    has_data = visited = on_edge = visited_2 = visited_3 = false;
    group_id = -1;
    max_color = 0;
}

Group::Group(int _id) {
    id = _id;
    point_count = edge_pointcnt = 0;
    has_info = false;
    center_x = center_y = center_z = 0.0;
    min_refl = 1000.0;
    max_refl = 0.0;
    avg_refl = 0.0;
    dev_refl = 0.0;
    approx_length = edge_length = 2000.0;
}

void Group::calc_metrics(Classifier & c) {
    if (has_info)
        return;
    has_info = true;
    if (point_count == 1) {
        length = max_width = max_height = 0;
        act_width = act_height = 0;
        approx_length = 0;
        direction.x = 0;
        direction.y = 0;
        direction.z = 0;
        cerr << "Error: Failed to calculate metrics due to low point count!" << endl;
        return;
    }
    float x1, y1, z1, x2, y2, z2;
    float max_dist = 0.0;
    int selected_num = min(POINT_THRE, (int)edge_data.size());
    vector<ObjectPoint> selected_data(POINT_THRE);
    sample(edge_data.begin(), edge_data.end(), selected_data.begin(), POINT_THRE, mt19937{random_device{}()});
    for (int i = 0; i < selected_num; i++)
        for (int j = 0; j < selected_num; j++) {
            if (i == j)
                continue;
            float dist = sqrt(
                square(selected_data[i].x - selected_data[j].x) +
                square(selected_data[i].y - selected_data[j].y) +
                square(selected_data[i].z - selected_data[j].z)
            );
            if (dist > max_dist) {
                max_dist = dist;
                x1 = selected_data[i].x;
                y1 = selected_data[i].y;
                z1 = selected_data[i].z;
                x2 = selected_data[j].x;
                y2 = selected_data[j].y;
                z2 = selected_data[j].z;
            }
        }
    length = max_dist;
    if (x2 - x1 > 0) {
        direction.x = x2 - x1;
        direction.y = y2 - y1;
        direction.z = z2 - z1;
    }
    else {
        direction.x = x1 - x2;
        direction.y = y1 - y2;
        direction.z = z1 - z2;
    }
    float curr_max_width = 0.0;
    float curr_act_width = 0.0;
    bool width_succ = false;
    float width_dx, width_dy, width_dz;
    float act_width_dx, act_width_dy, act_width_dz;
    for (int i = 0; i < selected_num; i++)
        for (int j = 0; j < selected_num; j++) {
            if (i == j)
                continue;
            float width = sqrt(
                square(selected_data[i].x - selected_data[j].x) +
                square(selected_data[i].y - selected_data[j].y) +
                square(selected_data[i].z - selected_data[j].z)
            );
            float inner_prod = 
                direction.x * (selected_data[i].x - selected_data[j].x) + 
                direction.y * (selected_data[i].y - selected_data[j].y) + 
                direction.z * (selected_data[i].z - selected_data[j].z);
            float cos_angle = fabs(inner_prod / (width * length));
            float sin_angle = sqrt(1 - square(cos_angle));
            float adj_width = width * sin_angle;
            if (cos_angle <= ANGLE_THRE_1 && adj_width > curr_act_width) {
                width_succ = true;
                curr_act_width = adj_width;
                float len = width * cos_angle;
                if (inner_prod >= 0) {
                    act_width_dx = selected_data[i].x - selected_data[j].x - (len * direction.x / length);
                    act_width_dy = selected_data[i].y - selected_data[j].y - (len * direction.y / length);
                    act_width_dz = selected_data[i].z - selected_data[j].z - (len * direction.z / length);
                }
                else {
                    act_width_dx = selected_data[i].x - selected_data[j].x + (len * direction.x / length);
                    act_width_dy = selected_data[i].y - selected_data[j].y + (len * direction.y / length);
                    act_width_dz = selected_data[i].z - selected_data[j].z + (len * direction.z / length);
                }
            }
            if (adj_width > curr_max_width) {
                curr_max_width = adj_width;
                float len = width * cos_angle;
                if (inner_prod >= 0) {
                    width_dx = selected_data[i].x - selected_data[j].x - (len * direction.x / length);
                    width_dy = selected_data[i].y - selected_data[j].y - (len * direction.y / length);
                    width_dz = selected_data[i].z - selected_data[j].z - (len * direction.z / length);
                }
                else {
                    width_dx = selected_data[i].x - selected_data[j].x + (len * direction.x / length);
                    width_dy = selected_data[i].y - selected_data[j].y + (len * direction.y / length);
                    width_dz = selected_data[i].z - selected_data[j].z + (len * direction.z / length);
                }
            }
        }
    if (!width_succ)
        cerr << "Error: Failed to calculate actual width!" << endl;
    act_width = curr_act_width;
    max_width = curr_max_width;
    float curr_act_height = 0.0;
    float curr_act_height_approx = 0.0;
    float curr_max_height = 0.0;
    bool height_succ = false;
    bool height_approx_succ = false;
    for (int i = 0; i < selected_num; i++)
        for (int j = 0; j < selected_num; j++) {
            if (i == j)
                continue;
            float height = sqrt(
                square(selected_data[i].x - selected_data[j].x) +
                square(selected_data[i].y - selected_data[j].y) +
                square(selected_data[i].z - selected_data[j].z)
            );
            float inner_prod_length = 
                direction.x * (selected_data[i].x - selected_data[j].x) + 
                direction.y * (selected_data[i].y - selected_data[j].y) + 
                direction.z * (selected_data[i].z - selected_data[j].z);
            float cos_angle_length = fabs(inner_prod_length / (height * length));
            float length_coord = height * cos_angle_length;
            if (width_succ) {
                float inner_prod_act_width = 
                    act_width_dx * (selected_data[i].x - selected_data[j].x) + 
                    act_width_dy * (selected_data[i].y - selected_data[j].y) + 
                    act_width_dz * (selected_data[i].z - selected_data[j].z);
                float cos_angle_act_width = fabs(inner_prod_act_width / (height * act_width));
                float act_width_coord = height * cos_angle_act_width;
                float cos_angle_act = sqrt(square(length_coord) + square(act_width_coord)) / height;
                float sin_angle_act = sqrt(1 - square(cos_angle_act));
                float adj_act_height = height * sin_angle_act;
                if (cos_angle_act <= ANGLE_THRE_2 && adj_act_height > curr_act_height) {
                    height_succ = true;
                    curr_act_height = adj_act_height;
                }
                else if (cos_angle_act <= ANGLE_THRE_3 && adj_act_height > curr_act_height_approx) {
                    height_approx_succ = true;
                    curr_act_height_approx = adj_act_height;
                }
            }
            float inner_prod_width = 
                width_dx * (selected_data[i].x - selected_data[j].x) + 
                width_dy * (selected_data[i].y - selected_data[j].y) + 
                width_dz * (selected_data[i].z - selected_data[j].z);
            float cos_angle_width = fabs(inner_prod_width / (height * max_width));
            float width_coord = height * cos_angle_width;
            float cos_angle = sqrt(square(length_coord) + square(width_coord)) / height;
            float sin_angle = sqrt(1 - square(cos_angle));
            float adj_height = height * sin_angle;
            if (adj_height > curr_max_height)
                curr_max_height = adj_height;
        }
    if (!height_succ && !height_approx_succ)
        cerr << "Error: Failed to calculate actual height!" << endl;
    if (height_succ)
        act_height = curr_act_height;
    else
        act_height = curr_act_height_approx;
    max_height = curr_max_height;
    bulk = length * act_width * act_height;
    int start_x = (round(x1 / c.dist) - c.x_min);
    int start_y = (round(y1 / c.dist) - c.y_min);
    int start_z = (round(z1 / c.dist) - c.z_min);
    int end_x = (round(x2 / c.dist) - c.x_min);
    int end_y = (round(y2 / c.dist) - c.y_min);
    int end_z = (round(z2 / c.dist) - c.z_min);
    int offset_start = c.get_offset(start_x, start_y, start_z);
    int offset_end = c.get_offset(end_x, end_y, end_z);
    if (!c.grid[offset_start].on_edge) {
        cerr << "Error: Start point not on edge!" << endl;
        return;
    }
    if (!c.grid[offset_end].on_edge) {
        cerr << "Error: End point not on edge!" << endl;
        return;
    }
    const char tmp[27][3] = {
        0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1, 0, 1, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 1, 0, 1, -1, 
        0, -1, 1, 0, -1, -1, -1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0, -1, 1, 1, 0, 1, -1, 0, -1, 1, 0, 
        -1, -1, 0, 1, 1, 1, 1, 1, -1, 1, -1, 1, 1, -1, -1, -1, 1, 1, -1, 1, -1, -1, -1, 1, -1, -1, -1
    };
    TriplePair<float> start_repr_old = c.grid[offset_start].repr_point;
    c.grid[offset_start].repr_point = TriplePair<float>(x1, y1, z1);
    TriplePair<float> end_repr_old = c.grid[offset_end].repr_point;
    c.grid[offset_end].repr_point = TriplePair<float>(x2, y2, z2);
    queue<pair<TriplePair<int>, float> > q;
    q.push(make_pair(TriplePair<int>(start_x, start_y, start_z), 0.0));
    bool found = false;
    while(!q.empty()) {
        TriplePair<int> curr_pos = q.front().first;
        float curr_length = q.front().second;
        q.pop();
        if (curr_pos.x == end_x && curr_pos.y == end_y && curr_pos.z == end_z) {
            found = true;
            approx_length = curr_length;
            break;
        }
        int offset = c.get_offset(curr_pos.x, curr_pos.y, curr_pos.z);
        if (c.grid[offset].visited_2)
            continue;
        c.grid[offset].visited_2 = true;
        for (int i = 0; i < 27; i++) {
            int dx = tmp[i][0];
            int dy = tmp[i][1];
            int dz = tmp[i][2];
            if (curr_pos.x + dx >= 0 && curr_pos.x + dx < c.x_len && 
                    curr_pos.y + dy >= 0 && curr_pos.y + dy < c.y_len && 
                    curr_pos.z + dz >= 0 && curr_pos.z + dz < c.z_len) {
                int offset_next = c.get_offset(curr_pos.x + dx, curr_pos.y + dy, curr_pos.z + dz);
                if (c.grid[offset_next].on_edge && c.grid[offset_next].has_data && 
                        !c.grid[offset_next].visited_2) {
                    float add_length = sqrt(
                        square(c.grid[offset].repr_point.x - c.grid[offset_next].repr_point.x) + 
                        square(c.grid[offset].repr_point.y - c.grid[offset_next].repr_point.y) + 
                        square(c.grid[offset].repr_point.z - c.grid[offset_next].repr_point.z)
                    );
                    q.push(make_pair(
                        TriplePair<int>(curr_pos.x + dx, curr_pos.y + dy, curr_pos.z + dz),
                        curr_length + add_length
                    ));
                }
            }
        }
    }
    c.grid[offset_start].repr_point = start_repr_old;
    c.grid[offset_end].repr_point = end_repr_old;
    if (!found)
        cerr << "Error: Failed to calculate approximate length!" << endl;
    TriplePair<float> start_center_old = c.grid[offset_start].center_point;
    c.grid[offset_start].center_point = TriplePair<float>(x1, y1, z1);
    TriplePair<float> end_center_old = c.grid[offset_end].center_point;
    c.grid[offset_end].center_point = TriplePair<float>(x2, y2, z2);
    queue<pair<TriplePair<int>, float> > q2;
    q2.push(make_pair(TriplePair<int>(start_x, start_y, start_z), 0.0));
    bool found2 = false;
    while(!q2.empty()) {
        TriplePair<int> curr_pos = q2.front().first;
        float curr_length = q2.front().second;
        q2.pop();
        if (curr_pos.x == end_x && curr_pos.y == end_y && curr_pos.z == end_z) {
            found2 = true;
            edge_length = curr_length;
            break;
        }
        int offset = c.get_offset(curr_pos.x, curr_pos.y, curr_pos.z);
        if (c.grid[offset].visited_3)
            continue;
        c.grid[offset].visited_3 = true;
        for (int i = 0; i < 27; i++) {
            int dx = tmp[i][0];
            int dy = tmp[i][1];
            int dz = tmp[i][2];
            if (curr_pos.x + dx >= 0 && curr_pos.x + dx < c.x_len && 
                    curr_pos.y + dy >= 0 && curr_pos.y + dy < c.y_len && 
                    curr_pos.z + dz >= 0 && curr_pos.z + dz < c.z_len) {
                int offset_next = c.get_offset(curr_pos.x + dx, curr_pos.y + dy, curr_pos.z + dz);
                if (c.grid[offset_next].on_edge && c.grid[offset_next].has_data && 
                        !c.grid[offset_next].visited_3) {
                    float add_length = sqrt(
                        square(c.grid[offset].center_point.x - c.grid[offset_next].center_point.x) + 
                        square(c.grid[offset].center_point.y - c.grid[offset_next].center_point.y) + 
                        square(c.grid[offset].center_point.z - c.grid[offset_next].center_point.z)
                    );
                    q2.push(make_pair(
                        TriplePair<int>(curr_pos.x + dx, curr_pos.y + dy, curr_pos.z + dz),
                        curr_length + add_length
                    ));
                }
            }
        }
    }
    c.grid[offset_start].center_point = start_center_old;
    c.grid[offset_end].center_point = end_center_old;
    if (!found2)
        cerr << "Error: Failed to calculate edge center length!" << endl;
}

void Group::print_metrics() {
    if (!has_info) {
        cerr << "Error: Function calc_metrics() must be called before calling print_metrics()!" << endl;
        return;
    }
    printf("Metrics of Group %d:\n", id + 1);
    printf("  Number of points: %d(total), %d(edge)\n", point_count, edge_pointcnt);
    printf("  Position: (%.3f, %.3f, %.3f), Direction: (%.3f, %.3f, %.3f)\n", 
        center_x, center_y, center_z, direction.x, direction.y, direction.z);
    printf("  Max. Length: %.3f m, Max. Width: %.3f m, Max. Height: %.3f m, Bulk: %.3f\n", 
        length, max_width, max_height, bulk);
    printf("  Act. Length: %.3f m(repr.) %.3f m(cent.), Act. Width: %.3f m, Act. Height: %.3f m\n", 
        approx_length, edge_length, act_width, act_height);
    printf("  Avg. Refl.: %.3f, Min. Refl.: %.3f, Max. Refl.: %.3f, Std.Dev. Refl.: %.3f\n", 
        avg_refl, min_refl, max_refl, dev_refl);
}

Classifier::Classifier(float _dist, float _pointcnt) {
    x_min = y_min = z_min = INT_MAX;
    x_max = y_max = z_max = INT_MIN;
    group_count = 0;
    grid = NULL;
    dist = _dist;
    pointcnt = _pointcnt;
}

Classifier::~Classifier() {
    delete [] grid;
}

void Classifier::init(vector<Point> & rawdata) {
    vector<Point>::iterator it;
    for (it = rawdata.begin(); it != rawdata.end(); it++) {
        int x = round(it->x / dist);
        int y = round(it->y / dist);
        int z = round(it->z / dist);
        if (x < x_min)
            x_min = x;
        if (x > x_max)
            x_max = x;
        if (y < y_min)
            y_min = y;
        if (y > y_max)
            y_max = y;
        if (z < z_min)
            z_min = z;
        if (z > z_max)
            z_max = z;
    }
    x_len = x_max - x_min + 1;
    y_len = y_max - y_min + 1;
    z_len = z_max - z_min + 1;
    grid = new GridElement[x_len * y_len * z_len];
    for (it = rawdata.begin(); it != rawdata.end(); it++) {
        if (it->group_id != -1) {
            int offset = get_offset(it->x, it->y, it->z);
            grid[offset].has_data = true;
            if (it->color > grid[offset].max_color) {
                grid[offset].max_color = it->color;
                grid[offset].repr_point = TriplePair<float>(it->x, it->y, it->z);
            }
        }
    }
    for (int i = 0; i < x_len; i++)
        for (int j = 0; j < y_len; j++)
            for (int k = 0; k < z_len; k++) {
                int offset = get_offset(i, j, k);
                grid[offset].center_point = TriplePair<float>(
                    (x_min + i) * dist, 
                    (y_min + j) * dist, 
                    (z_min + k) * dist
                );
            }
}

int Classifier::group_data(vector<Point> & rawdata) {
    init(rawdata);
    for (int i = 0; i < x_len; i++)
        for (int j = 0; j < y_len; j++)
            for (int k = 0; k < z_len; k++) {
                int offset = get_offset(i, j, k);
                if (grid[offset].has_data && !grid[offset].visited) {
                    Group group(group_count);
                    group_list.push_back(group);
                    alloc_groupid(i, j, k, group_count);
                    group_count++;
                }
            }
    vector<Point>::iterator it;
    for (it = rawdata.begin(); it != rawdata.end(); it++) {
        if (it->group_id != -1) {
            int offset = get_offset(it->x, it->y, it->z);
            it->group_id = grid[offset].group_id;
            if (grid[offset].on_edge) {
                group_list[it->group_id].edge_data.push_back(ObjectPoint(it->x, it->y, it->z, it->color, it->r));
                if (it->is_origin)
                    group_list[it->group_id].edge_pointcnt++;
            }
            if (it->is_origin) {
                group_list[it->group_id].origin_data.push_back(ObjectPoint(it->x, it->y, it->z, it->color, it->r));
                group_list[it->group_id].center_x += it->x;
                group_list[it->group_id].center_y += it->y;
                group_list[it->group_id].center_z += it->z;
                group_list[it->group_id].avg_refl += it->color;
                if (it->color > group_list[it->group_id].max_refl)
                    group_list[it->group_id].max_refl = it->color;
                if (it->color < group_list[it->group_id].min_refl)
                    group_list[it->group_id].min_refl = it->color;
                group_list[it->group_id].point_count++;
            }
        }
    }
    for (int i = 0; i < group_count; i++) {
        group_list[i].id = i;
        group_list[i].center_x /= group_list[i].point_count;
        group_list[i].center_y /= group_list[i].point_count;
        group_list[i].center_z /= group_list[i].point_count;
        group_list[i].avg_refl /= group_list[i].point_count;
    }
    for (it = rawdata.begin(); it != rawdata.end(); it++) {
        if (it->group_id != -1 && it->is_origin)
            group_list[it->group_id].dev_refl += square(it->color - group_list[it->group_id].avg_refl);
    }
    for (int i = 0; i < group_count; i++) {
        if (group_list[i].point_count <= 1)
            group_list[i].dev_refl = 0;
        else
            group_list[i].dev_refl = sqrt(group_list[i].dev_refl / (group_list[i].point_count - 1));
    }
    sort(group_list.begin(), group_list.end(), GroupComp(pointcnt));
    vector<int> map_id(group_count);
    int curr_id = 0;
    for (int i = 0; i < group_count; i++) {
        if (group_list[i].point_count >= pointcnt) {
            map_id[group_list[i].id] = curr_id;
            group_list[i].id = curr_id;
            curr_id++;
        }
        else {
            map_id[group_list[i].id] = -1;
            group_list[i].id = -1;
        }
    }
    for (it = rawdata.begin(); it != rawdata.end(); it++) {
        if (it->group_id != -1)
            it->group_id = map_id[it->group_id];
    }
    return curr_id;
}

int Classifier::get_offset(float x, float y, float z) {
    int offset = 0;
    offset += (round(z / dist) - z_min);
    offset += (round(y / dist) - y_min) * z_len;
    offset += (round(x / dist) - x_min) * y_len * z_len;
    return offset;
}

int Classifier::get_offset(int x, int y, int z) {
    int offset = 0;
    offset += z;
    offset += y * z_len;
    offset += x * y_len * z_len;
    return offset;
}

void Classifier::alloc_groupid(int x, int y, int z, int id) {
    queue<TriplePair<int> > q;
    q.push(TriplePair<int>(x, y, z));
    while(!q.empty()) {
        TriplePair<int> curr = q.front();
        q.pop();
        int offset = get_offset(curr.x, curr.y, curr.z);
        if (!grid[offset].has_data || grid[offset].visited)
            continue;
        grid[offset].visited = true;
        grid[offset].group_id = id;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
                for (int dz = -1; dz <= 1; dz++) {
                    if (curr.x + dx >= 0 && curr.x + dx < x_len && 
                            curr.y + dy >= 0 && curr.y + dy < y_len && 
                            curr.z + dz >= 0 && curr.z + dz < z_len && 
                            grid[get_offset(curr.x + dx, curr.y + dy, curr.z + dz)].has_data)
                        q.push(TriplePair<int>(curr.x + dx, curr.y + dy, curr.z + dz));
                    else if (abs(dx) + abs(dy) + abs(dz) == 1)
                        grid[offset].on_edge = true;
                }
    }
}

void Classifier::calc_groupinfo(int group_id) {
    group_list[group_id].calc_metrics(*this);
}