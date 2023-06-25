#include "core/components.h"
#include "core/color.h"
#include "core/database.h"

namespace core {

    void StringView::setData(const std::string& txt) {
        data = intern(txt);
        clean = intern(stripAnsi(txt));
    }

    nlohmann::json Destination::serialize() {
        nlohmann::json j;
        if(registry.valid(ent)) j["object"] = registry.get<ObjectId>(ent);
        auto room = std::get_if<RoomId>(&destination);
        if(room) {
            j["destination"] = *room;
        }
        auto grid = std::get_if<GridPoint>(&destination);
        if(grid) {
            j["destination"] = {grid->x, grid->y, grid->z};
        }
        auto sector = std::get_if<SectorPoint>(&destination);
        if(sector) {
            j["destination"] = {sector->x, sector->y, sector->z};
        }
        return j;
    }

    Destination::Destination(const nlohmann::json& j) {
        if(j.contains("object")) {
            ObjectId id(j["object"]);
            ent = id.getObject();
        }
        if(j.contains("destination")) {
            // if there's only one element in the array, it's a room id.
            auto dest = j["destination"];
            // if dest is a number, then it's a room.
            if(dest.is_number_integer()) {
                destination = dest.get<RoomId>();
            }
            else {
                // otherwise, it's a grid point or a sector point.
                if (dest[0].is_number_float()) {
                    // it's a sector.
                    destination = SectorPoint(dest);
                } else {
                    // it's a grid.
                    destination = GridPoint(dest);
                }
            }
        }
    }

}